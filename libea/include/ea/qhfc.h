/* qhfc.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _EA_QHFC_H_
#define _EA_QHFC_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <algorithm>

#include <ea/datafile.h>
#include <ea/metadata.h>
#include <ea/individual.h>
#include <ea/selection/elitism.h>
#include <ea/selection/random.h>
#include <ea/generational_models/crowding.h>
#include <ea/metapopulation.h>
#include <ea/evolutionary_algorithm.h>


namespace ealib {
    
    /*! QHFC evolutionary algorithm.
     
     Note: working on cleaning this up, but for right now, here's how to declare
     an instance of the QHFC algorithm:
     
     typedef meta_population<
     // embedded ea type:
     evolutionary_algorithm<bitstring,
     mutation::operators::per_site<mutation::site::bitflip>,
     all_ones,
     configuration,
     recombination::two_point_crossover,
     generational_models::deterministic_crowding< > >,
     // mp types:
     mutation::operators::no_mutation,
     constant,
     qhfc_configuration,
     recombination::no_recombination,
     generational_models::qhfc,
     attr::no_attributes> ea_type;
     */
    
    LIBEA_MD_DECL(QHFC_POP_SCALE, "ea.qhfc.population_scale", double);
    LIBEA_MD_DECL(QHFC_DETECT_EXPORT_NUM, "ea.qhfc.detect_export_num", double);
    LIBEA_MD_DECL(QHFC_CATCHUP_GEN, "ea.qhfc.catchup_gen", double);
    LIBEA_MD_DECL(QHFC_PERCENT_REFILL, "ea.qhfc.percent_refill", double);
    LIBEA_MD_DECL(QHFC_BREED_TOP_FREQ, "ea.qhfc.breed_top_freq", double);
    LIBEA_MD_DECL(QHFC_NO_PROGRESS_GEN, "ea.qhfc.no_progess_gen", double);
    
    // run time only:
    LIBEA_MD_DECL(QHFC_ADMISSION_LEVEL, "ea.qhfc.admission_level", double);
    LIBEA_MD_DECL(QHFC_LAST_PROGRESS_GEN, "ea.qhfc.last_progess_gen", double);
    LIBEA_MD_DECL(QHFC_LAST_PROGRESS_MAX, "ea.qhfc.last_progess_max", double);
    
	namespace generational_models {
		
		/*! QHFC generational model.
         
         Highest index is the "top" subpopulation.
         */
		struct qhfc {
            
            //! Initialize QHFC; set the admission levels, initial populations.
            template <typename EA>
			void initialize(EA& ea) {
                
                // sanity checks:
                if(ea.size() <= 2) {
                    throw bad_argument_exception("qhfc::initialize: metapopulation must have size > 2.");
                }
                
                if((static_cast<double>(get<POPULATION_SIZE>(ea)) * get<QHFC_PERCENT_REFILL>(ea)) < 1.0) {
                    throw bad_argument_exception("qhfc::initialize: population size * pct refill < 1.0");
                }
                
                // mean fitness over all pops --> admission fitness for bottom level, F
                using namespace boost::accumulators;
                accumulator_set<double, stats<tag::mean> > mpfit;
                
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    for(typename EA::subpopulation_type::iterator j=i->begin(); j!=i->end(); ++j) {
                        mpfit(static_cast<double>(ealib::fitness(*j,*i)));
                    }
                }
                double mean_fitness = mean(mpfit);
                
                // remove individuals w/ fitness < F (use the subpopulation's population type to get at the pointers):
                typename EA::subpopulation_type::population_type all;
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    for(typename EA::subpopulation_type::population_type::iterator j=i->population().begin(); j!=i->population().end(); ++j) {
                        if(ealib::fitness(**j,*i) >= mean_fitness) {
                            all.push_back(*j);
                        }
                    }
                    i->clear();
                }
                
                // spread individuals across subpops by fitness
                std::sort(all.begin(), all.end(), comparators::fitness<typename EA::subpopulation_type>(ea[0]));
                std::size_t spsize=all.size() / get<METAPOPULATION_SIZE>(ea);
                for(typename EA::reverse_iterator i=ea.rbegin(); i!=ea.rend(); ++i) { // subpop
                    generate_population(*i);
                    for(std::size_t j=0; j<spsize; ++j) {
                        (*i)[j] = *all.back(); // overwrite
                        put<QHFC_ADMISSION_LEVEL>(static_cast<double>(ealib::fitness((*i)[j],*i)), *i);
                        all.pop_back();
                    }
                }
                
                // clean up if there are extras, and set the admission level for sp0:
                std::copy(all.begin(), all.end(), ea[0].population().begin()+spsize);
                put<QHFC_ADMISSION_LEVEL>(mean_fitness, ea[0]);
                
                // finally, initialize last progress gen and fitness:
                put<QHFC_LAST_PROGRESS_GEN>(0.0, ea);
                put<QHFC_LAST_PROGRESS_MAX>(0.0, ea);
            }
            
            /*! Adjust the admission level of each subpopulation.
             */
            template <typename EA>
			void adjust_admission_levels(EA& ea) {
                using namespace boost::accumulators;
                accumulator_set<double, stats<tag::mean,tag::max> > topfit;
                
                for(typename EA::subpopulation_type::iterator j=ea.rbegin()->begin(); j!=ea.rbegin()->end(); ++j) { // individual
                    topfit(static_cast<double>(ealib::fitness(*j,*ea.rbegin())));
                }
                
                if(mean(topfit) > (2*get<QHFC_ADMISSION_LEVEL>(*ea.rbegin()) - get<QHFC_ADMISSION_LEVEL>(*(ea.rbegin()+1)))) {
                    // adjust level
                    double fmin = get<QHFC_ADMISSION_LEVEL>(ea[0]);
                    double fmax = max(topfit);
                    
                    for(std::size_t i=1; i<ea.size(); ++i) {
                        double fkadm = fmin + i*(fmax-fmin)/static_cast<double>(ea.size());
                        put<QHFC_ADMISSION_LEVEL>(fkadm, ea[i]);
                    }
                }
            }
            
            /*! Recursively import individuals from f+1 into f until l is reached.
             */
            template <typename ForwardIterator, typename EA>
			typename EA::subpopulation_type::population_type import_from_below(ForwardIterator f, ForwardIterator l, std::size_t n, EA& ea) {
                // select n random individuals to export, remove them from this population:
                typename EA::subpopulation_type::population_type exports;
                algorithm::random_split(f->population(), exports, n, ea.rng());
                
                if((f+1) == l) {
                    // bottom population; need to fill up f's population:
                    generate_population(*f);
                } else {
                    // not at the bottom, need to import
                    typename EA::subpopulation_type::population_type imports = import_from_below(f+1, l, n, ea);
                    f->insert(f->end(), imports.begin(), imports.end());
                }
                
                return exports;
            }
            
            /*! Do potency testing on the i'th subpopulation.
             t == subpopulation "above" (higher fitness)
             f == the one we're checking
             l == the end of the road
             */
            template <typename ForwardIterator, typename EA>
            bool potency_testing(ForwardIterator t, ForwardIterator f, ForwardIterator l, EA& ea) {
                int catchup_eval=0;
                typename EA::subpopulation_type::population_type exports;

                while((catchup_eval++ < (get<QHFC_CATCHUP_GEN>(ea)*f->size())) && (exports.size() < get<QHFC_DETECT_EXPORT_NUM>(ea))) {
                    // grab two parents at random and perform deterministic crowding:
                    typename EA::subpopulation_type::population_type pop;
                    algorithm::random_split(f->population(), pop, 2, ea.rng());
                    generational_models::deterministic_crowding< > dc;
                    dc(pop, *f);
                    
                    // now, see if any individuals in pop have fitness > than the
                    // admission level of the next higest subpop:
                    for(typename EA::subpopulation_type::population_type::iterator j=pop.begin(); j!=pop.end(); ++j) {
                        if((ealib::fitness(**j,*f) > get<QHFC_ADMISSION_LEVEL>(*t))
                           && (exports.size() < get<QHFC_DETECT_EXPORT_NUM>(ea))) {
                            // promote
                            exports.push_back(*j);
                            typename EA::subpopulation_type::population_type imports = import_from_below(f+1, l, 1, ea);
                            f->insert(f->end(), imports.begin(), imports.end());
                        } else {
                            // keep
                            f->insert(f->end(), *j);
                        }
                    }
                }
                
                // detect (im)potency
                bool potent = (exports.size() >= get<QHFC_DETECT_EXPORT_NUM>(ea));

                // export the exports to i+1 subpopulation
                typename EA::subpopulation_type::population_type next;
                selection::elitism<selection::random< > > sel(t->size()-exports.size(), t->population(), t);
                sel(t->population(), next, t->size()-exports.size(), *t);
                next.insert(next.end(), exports.begin(), exports.end());
                std::swap(t->population(), next);

                return potent;
            }
            
            /*! Breed the top population.
             
             Update the top population BREED_TOP_FREQ number of updates, tracking the
             maximum fitness level.  If the max fitness level doesn't change for
             LAST_PROGRESS_GEN, then we call import_from_below().
             */
            template <typename EA>
            void breed_top(EA& ea) {
                using namespace boost::accumulators;
                typename EA::subpopulation_type& top=*ea.rbegin();
                
                for(std::size_t i=0; i<get<QHFC_BREED_TOP_FREQ>(ea); ++i) {
                    top.update();
                    accumulator_set<double, stats<tag::max> > spfit;
                    for(typename EA::subpopulation_type::iterator j=top.begin(); j!=top.end(); ++j) { // individual
                        spfit(static_cast<double>(ealib::fitness(*j,top)));
                    }
                    if(max(spfit) > get<QHFC_LAST_PROGRESS_MAX>(ea)) {
                        put<QHFC_LAST_PROGRESS_GEN>(top.current_update(), ea);
                        put<QHFC_LAST_PROGRESS_MAX>(max(spfit), ea);
                    }
                    if((top.current_update() - get<QHFC_LAST_PROGRESS_GEN>(ea)) >= get<QHFC_NO_PROGRESS_GEN>(ea)) {
                        typename EA::subpopulation_type::population_type imports = import_from_below(ea.rbegin()+1,
                                                                                    ea.rend(),
                                                                                    static_cast<std::size_t>(get<QHFC_PERCENT_REFILL>(ea)*top.size()),
                                                                                    ea);
                        typename EA::subpopulation_type::population_type next;
                        selection::elitism<selection::random< > > sel(top.size()-imports.size(), top.population(), top);
                        sel(top.population(), next, top.size()-imports.size(), top);
                        next.insert(next.end(), imports.begin(), imports.end());
                        std::swap(top.population(), next);
                    }
                }
            }
            
            //! Apply the QHFC generational model to the meta-population EA.
            template <typename Population, typename EA>
            void operator()(Population& population, EA& ea) {
                if(ea.current_update() == 0) {
                    initialize(ea);
                }
                
                breed_top(ea);
                adjust_admission_levels(ea);
                
                typename EA::reverse_iterator t=ea.rbegin(), i=ea.rbegin()+1;
                for( ; (i+1)!=ea.rend(); ++t, ++i) {
                    if(!potency_testing(t, i, ea.rend(), ea)) {
                        typename EA::subpopulation_type::population_type imports = import_from_below(i+1,
                                                                                    ea.rend(),
                                                                                    static_cast<std::size_t>(get<QHFC_PERCENT_REFILL>(*i)*i->size()),
                                                                                    ea);
                        std::random_shuffle(i->begin(), i->end(), ea.rng());
                        i->population().resize(get<POPULATION_SIZE>(*i)-imports.size());
                        i->insert(i->end(), imports.begin(), imports.end());
                        i->update();
                    }
                }
            }
        };
        
    } // generational_models

    //! Lifecycle object for QHFC.
    struct qhfc_lifecycle : default_lifecycle {
        //! Called as the final step of EA initialization.
        template <typename EA>
        void initialize(EA& ea) {
            // set the population sizes of the various different subpopulations:
            std::size_t base_size=get<POPULATION_SIZE>(ea);
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                put<POPULATION_SIZE>(base_size, *i);
                base_size *= get<QHFC_POP_SCALE>(ea);
            }
        }
    };

    
    //! QHFC evolutionary algorithm definition.
    template
    < typename Representation
    , typename FitnessFunction
	, typename MutationOperator
	, typename RecombinationOperator
    , typename AncestorGenerator
    , typename StopCondition=dont_stop
    , typename Lifecycle=default_lifecycle
    > class qhfc
    : public metapopulation
    < evolutionary_algorithm // begin subpopulation
    < Representation
    , FitnessFunction
    , MutationOperator
    , RecombinationOperator
    , generational_models::deterministic_crowding< >
    , AncestorGenerator
    , dont_stop
    , fill_population
    , Lifecycle
    > // end subpopulation
    , quiet_nan
    , mutation::operators::no_mutation
    , recombination::no_recombination
    , generational_models::qhfc
    , ancestors::default_subpopulation
    , StopCondition
    , fill_metapopulation
    , qhfc_lifecycle
    > {
    public:
        typedef metapopulation
        < evolutionary_algorithm // begin subpopulation
        < Representation
        , FitnessFunction
        , MutationOperator
        , RecombinationOperator
        , generational_models::deterministic_crowding< >
        , AncestorGenerator
        , dont_stop
        , fill_population
        , Lifecycle
        > // end subpopulation
        , quiet_nan
        , mutation::operators::no_mutation
        , recombination::no_recombination
        , generational_models::qhfc
        , ancestors::default_subpopulation
        , StopCondition
        , fill_metapopulation
        , qhfc_lifecycle
        > parent_type;
        
        //! Default constructor.
        qhfc() {
        }
        
        //! Initializing constructor.
        qhfc(const metadata& md) : parent_type(md) {
        }
    };
    

    namespace datafiles {
        
        //! QHFC datafile.
        template <typename EA>
        struct qhfc_dat : record_statistics_event<EA> {
            qhfc_dat(EA& ea)
            : record_statistics_event<EA>(ea)
            , _fitness("qhfc_fitness.dat")
            , _admission("qhfc_admission.dat")
            , _pop_size("qhfc_pop_size.dat"){
                _fitness.add_field("update");
                _admission.add_field("update");
                _pop_size.add_field("update");
                
                for(std::size_t i=0; i<get<METAPOPULATION_SIZE>(ea); ++i) {
                    _fitness.add_field("max_fitness_sp" + boost::lexical_cast<std::string>(i));
                    _fitness.add_field("mean_fitness_sp" + boost::lexical_cast<std::string>(i));
                    _fitness.add_field("min_fitness_sp" + boost::lexical_cast<std::string>(i));
                    _admission.add_field("admission_level_sp" + boost::lexical_cast<std::string>(i));
                    _pop_size.add_field("pop_size_sp" + boost::lexical_cast<std::string>(i));
                }
            }
            
            virtual ~qhfc_dat() {
            }
            
            virtual void operator()(EA& ea) {
                using namespace boost::accumulators;
                
                _fitness.write(ea.current_update());
                _admission.write(ea.current_update());
                _pop_size.write(ea.current_update());
                
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    accumulator_set<double, stats<tag::mean, tag::min, tag::max> > fit;
                    
                    for(typename EA::subpopulation_type::iterator j=i->begin(); j!=i->end(); ++j) {
                        fit(static_cast<double>(ealib::fitness(*j,*i)));
                    }
                    
                    _fitness.write(max(fit)).write(mean(fit)).write(min(fit));
                    _admission.write(get<QHFC_ADMISSION_LEVEL>(*i,0.0));
                    _pop_size.write(i->size());
                }
                
                _fitness.endl();
                _admission.endl();
                _pop_size.endl();
            }
            
            datafile _fitness;
            datafile _admission;
            datafile _pop_size;
        };
        
    } // datafiles
} // ealib

#endif
