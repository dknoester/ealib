/* fitness.h
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
#ifndef _EA_DATAFILES_FITNESS_H_
#define _EA_DATAFILES_FITNESS_H_

#include <boost/lexical_cast.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <ea/datafile.h>
#include <ea/attributes.h>
#include <ea/meta_population.h>


namespace ealib {
    namespace datafiles {
        
        /*! Datafile for mean generation, and mean & max fitness.
         */
        template <typename EA>
        struct fitness : record_statistics_event<EA> {
            fitness(EA& ea) : record_statistics_event<EA>(ea), _df("fitness.dat") {
                _df.add_field("update")
                .add_field("mean_generation")
                .add_field("min_fitness")
                .add_field("mean_fitness")
                .add_field("max_fitness");
            }
            
            virtual ~fitness() {
            }
            
            virtual void operator()(EA& ea) {
                using namespace boost::accumulators;
                accumulator_set<double, stats<tag::mean> > gen;
                accumulator_set<double, stats<tag::min, tag::mean, tag::max> > fit;
                
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    gen(get<IND_GENERATION>(*i));
                    fit(static_cast<double>(ealib::fitness(*i,ea)));
                }
                
                _df.write(ea.current_update())
                .write(mean(gen))
                .write(min(fit))
                .write(mean(fit))
                .write(max(fit))
                .endl();
            }
            
            datafile _df;
        };

        
        /*! Datafile for mean generation, and mean & max fitness.
         */
        template <typename EA>
        struct meta_population_fitness : record_statistics_event<EA> {
            meta_population_fitness(EA& ea)
            : record_statistics_event<EA>(ea)
            , _df("sub_population_fitness.dat")
            , _mp("meta_population_fitness.dat") {
                _df.add_field("update");
                for(std::size_t i=0; i<get<META_POPULATION_SIZE>(ea); ++i) {
                    _df.add_field("mean_generation_sp" + boost::lexical_cast<std::string>(i))
                    .add_field("min_fitness_sp" + boost::lexical_cast<std::string>(i))
                    .add_field("mean_fitness_sp" + boost::lexical_cast<std::string>(i))
                    .add_field("max_fitness_sp" + boost::lexical_cast<std::string>(i));
                }
                
                _mp.add_field("update")
                .add_field("mean_size")
                .add_field("mean_generation")
                .add_field("min_fitness")
                .add_field("mean_fitness")
                .add_field("max_fitness");
            }
            
            virtual ~meta_population_fitness() {
            }
            
            virtual void operator()(EA& ea) {
                using namespace boost::accumulators;

                accumulator_set<double, stats<tag::mean> > mpsize;
                accumulator_set<double, stats<tag::mean> > mpgen;
                accumulator_set<double, stats<tag::min, tag::mean, tag::max> > mpfit;
                
                _df.write(ea.current_update());
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    accumulator_set<double, stats<tag::mean> > gen;
                    accumulator_set<double, stats<tag::min, tag::mean, tag::max> > fit;
                    
                    for(typename EA::individual_type::iterator j=i->begin(); j!=i->end(); ++j) {
                        gen(get<IND_GENERATION>(*j));
                        fit(static_cast<double>(ealib::fitness(*j,*i)));
                        mpsize(i->size());
                        mpgen(get<IND_GENERATION>(*j));
                        mpfit(static_cast<double>(ealib::fitness(*j,*i)));
                    }
                    
                    _df.write(mean(gen))
                    .write(min(fit))
                    .write(mean(fit))
                    .write(max(fit));
                }
                _df.endl();
                
                _mp.write(ea.current_update())
                .write(mean(mpsize))
                .write(mean(mpgen))
                .write(min(mpfit))
                .write(mean(mpfit))
                .write(max(mpfit))
                .endl();
            }
            
            datafile _df;
            datafile _mp;
        };
                
    } // datafiles
} // ea

#endif
