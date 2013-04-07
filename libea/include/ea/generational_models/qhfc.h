/* qhfc.h
 *
 * This file is part of EALib.
 *
 * Copyright 2012 David B. Knoester.
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
#ifndef _EA_GENERATIONAL_MODELS_QHFC_H_
#define _EA_GENERATIONAL_MODELS_QHFC_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <algorithm>
#include <ea/meta_data.h>
#include <ea/generational_model.h>
#include <ea/selection/elitism.h>
#include <ea/selection/random.h>
#include <ea/generational_models/crowding.h>

LIBEA_MD_DECL(QHFC_DETECT_EXPORT_NUM, "ea.qhfc.detect_export_num", double);
LIBEA_MD_DECL(QHFC_CATCHUP_GEN, "ea.qhfc.catchup_gen", double);
LIBEA_MD_DECL(QHFC_PERCENT_REFILL, "ea.qhfc.percent_refill", double);
LIBEA_MD_DECL(QHFC_BREED_TOP_FREQ, "ea.qhfc.breed_top_freq", double);
LIBEA_MD_DECL(QHFC_NO_PROGRESS_GEN, "ea.qhfc.no_progess_gen", double);

// run time only:
LIBEA_MD_DECL(QHFC_ADMISSION_LEVEL, "ea.qhfc.admission_level", double);
LIBEA_MD_DECL(QHFC_LAST_PROGRESS_GEN, "ea.qhfc.last_progess_gen", double);
LIBEA_MD_DECL(QHFC_LAST_PROGRESS_MAX, "ea.qhfc.last_progess_max", double);


namespace ealib {
	namespace generational_models {
		
		/*! QHFC generational model.
         
         Highest index is the "top" subpopulation.
         */
		struct qhfc : public generational_model {
            
            //! Initialize QHFC; set the admission levels, initial populations.
            template <typename EA>
			void initialize(EA& ea) {
                // mean fitness over all pops --> admission fitness for bottom level, F
                using namespace boost::accumulators;
                accumulator_set<double, stats<tag::mean> > mpfit;
                
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) { // subpop
                    for(typename EA::individual_type::iterator j=i->begin(); j!=i->end(); ++j) { // individual
                        mpfit(static_cast<double>(ealib::fitness(*j,*i)));
                    }
                }
                double mean_fitness = mean(mpfit);
                
                // remove individuals w/ fitness < F
                typename EA::subpopulation_type all;
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) { // subpop
                    for(typename EA::individual_type::population_type::iterator j=i->population().begin(); j!=i->population().end(); ++j) { // individual
                        if(ealib::fitness(**j,*i) >= mean_fitness) {
                            all.push_back(*j);
                        }
                    }
                    i->clear();
                }
                
                // spread individuals across subpops by fitness
                std::sort(all.begin(), all.end(), comparators::fitness<typename EA::individual_type>(ea[0]));
                std::size_t spsize=all.size() / get<META_POPULATION_SIZE>(ea);
                for(typename EA::reverse_iterator i=ea.rbegin(); i!=ea.rend(); ++i) { // subpop
                    i->configuration().initial_population(*i); // refill...
                    for(std::size_t j=0; j<spsize; ++j) {
                        (*i)[j] = *all.back(); // overwrite
                        put<QHFC_ADMISSION_LEVEL>(static_cast<double>(ealib::fitness(*all.back(),*i)), *i);
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
                
                for(typename EA::individual_type::iterator j=ea.rbegin()->begin(); j!=ea.rbegin()->end(); ++j) { // individual
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
			typename EA::subpopulation_type import_from_below(ForwardIterator f, ForwardIterator l, std::size_t n, EA& ea) {
                // select n random individuals to export, remove them from this population:
                typename EA::subpopulation_type exports;
                algorithm::random_split(f->population(), exports, n, ea.rng());
                
                if((f+1) == l) {
                    // bottom population; need to generate the imports
                    f->configuration().fill_population(*f);
                } else {
                    // not at the bottom, need to import
                    typename EA::subpopulation_type imports = import_from_below(f+1, l, n, ea);
                    f->append(imports.begin(), imports.end());
                }

                return exports;
            }
            
            /*! Do potency testing on the i'th subpopulation.
             */
            template <typename EA>
            bool potency_testing(std::size_t i, EA& ea) {
                int catchup_eval=0;
                typename EA::subpopulation_type exports;
                
                while((catchup_eval++ < (get<QHFC_CATCHUP_GEN>(ea)*ea[i].size()))
                      && (exports.size() < get<QHFC_DETECT_EXPORT_NUM>(ea))) {

                    // grab two parents at random and perform deterministic crowding:
                    typename EA::subpopulation_type pop;

                    algorithm::random_split(ea[i].population(), pop, 2, ea.rng());
                    generational_models::deterministic_crowding< > dc;
                    dc(pop, ea[i]);
                    
                    // now, see if any individuals in pop have fitness > than the
                    // admission level of the next higest subpop:
                    for(typename EA::subpopulation_type::iterator j=pop.begin(); j!=pop.end(); ++j) {
                        // good fitness
                        if(ealib::fitness(**j,ea[i]) > get<QHFC_ADMISSION_LEVEL>(ea[i+1])) {
                            exports.push_back(*j);
                             // -2 to account for the riterator & next lowest level
                            typename EA::subpopulation_type imports = import_from_below(ea.rbegin() + (ea.size() - i - 2),
                                                                                        ea.rend(),
                                                                                        1, ea);
                            ea[i].append(imports.begin(), imports.end());
                            
                        } else { // not so good fitness
                            ea[i].append(*j);
                        }
                    }
                }
                // detect (im)potency
                bool potent = (exports.size() >= get<QHFC_DETECT_EXPORT_NUM>(ea));

                // export the exports to i+1 subpopulation
                typename EA::subpopulation_type next;
                selection::elitism<selection::random> sel(ea[i+1].size()-exports.size(), ea[i+1].population(), ea[i+1]);
                sel(ea[i+1].population(), next, ea[i+1].size()-exports.size(), ea[i+1]);
                next.insert(next.end(), exports.begin(), exports.end());
                std::swap(ea[i+1].population(), next);
                
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
                typename EA::individual_type& top=*ea.rbegin();

                for(std::size_t i=0; i<get<QHFC_BREED_TOP_FREQ>(ea); ++i) {
                    top.update();
                    accumulator_set<double, stats<tag::max> > spfit;
                    for(typename EA::individual_type::iterator j=top.begin(); j!=top.end(); ++j) { // individual
                        spfit(static_cast<double>(ealib::fitness(*j,top)));
                    }
                    if(max(spfit) > get<QHFC_LAST_PROGRESS_MAX>(ea)) {
                        put<QHFC_LAST_PROGRESS_GEN>(top.current_update(), ea);
                    }
                    if((top.current_update() - get<QHFC_LAST_PROGRESS_GEN>(ea)) >= get<QHFC_NO_PROGRESS_GEN>(ea)) {
                        typename EA::subpopulation_type imports = import_from_below(ea.rbegin(),
                                                                                    ea.rend(),
                                                                                    static_cast<std::size_t>(get<QHFC_PERCENT_REFILL>(ea)*top.size()), ea);
                        typename EA::subpopulation_type next;
                        selection::elitism<selection::random> sel(top.size()-imports.size(), top.population(), top);
                        sel(top.population(), next, top.size()-imports.size(), top);
                        next.insert(next.end(), imports.begin(), imports.end());
                        std::swap(top.population(), next);
                    }
                }
            }
            
            //! Apply the QHFC generational model to the meta-population EA.
            template <typename Population, typename EA>
            void operator()(Population& population, EA& ea) {
                BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
                BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
                
                if(ea.current_update() == 0) {
                    initialize(ea);
                }

                breed_top(ea);
                adjust_admission_levels(ea);
                
                assert(ea.size() >= 3);
                std::size_t idx=ea.size()-2;
                for(typename EA::reverse_iterator i=ea.rbegin()+1; (i+1)!=ea.rend(); ++i,--idx) { // subpop
                    if(!potency_testing(idx,ea)) {
                        typename EA::subpopulation_type imports = import_from_below(i+1, ea.rend(), static_cast<std::size_t>(get<QHFC_PERCENT_REFILL>(*i)*i->size()), ea);
                        std::random_shuffle(i->begin(), i->end(), ea.rng());
                        i->population().resize(get<POPULATION_SIZE>(*i)-imports.size());
                        i->append(imports.begin(), imports.end());
                        i->update();
                    }
                }
            }
        };
        
    } // generational_models
} // ea

#endif
