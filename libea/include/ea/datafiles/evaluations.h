/* evaluations.h
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
#ifndef _EA_DATAFILES_EVALUATIONS_H_
#define _EA_DATAFILES_EVALUATIONS_H_

#include <boost/lexical_cast.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <ea/datafile.h>
#include <ea/attributes.h>


namespace ealib {
    namespace datafiles {
                
        /*! Datafile for fitness evaluations; records population stats every 1000
         fitness evaluations.  Does not, itself, trigger fitness evaluations.
         */
        template <typename EA>
        struct fitness_evaluations : fitness_evaluated_event<EA> {
            
            fitness_evaluations(EA& ea) : fitness_evaluated_event<EA>(ea), _df("fitness_evaluations.dat"), _evals(0) {
                _df.add_field("evaluation")
                .add_field("mean_generation")
                .add_field("min_fitness")
                .add_field("mean_fitness")
                .add_field("max_fitness");
            }
            
            virtual ~fitness_evaluations() {
            }
            
            virtual void operator()(typename EA::individual_type& ind, EA& ea) {
                // record stats every 1000 fitness evaluations
                if((++_evals % 1000) == 0) {
                    using namespace boost::accumulators;
                    
                    accumulator_set<double, stats<tag::mean> > gen;
                    accumulator_set<double, stats<tag::min, tag::mean, tag::max> > fit;
                    
                    for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                        if(ealib::has_fitness(*i,ea)) {
                            gen(i->generation());
                            fit(static_cast<double>(ealib::fitness(*i,ea)));
                        }
                    }
                    
                    _df.write(_evals)
                    .write(mean(gen))
                    .write(min(fit))
                    .write(mean(fit))
                    .write(max(fit))
                    .endl();
                }
            }
            
            datafile _df;
            long _evals;
        };
        
        
        /*! Datafile for meta pop fitness evaluations.
         */
        template <typename EA>
        struct meta_population_fitness_evaluations : event {
            
            meta_population_fitness_evaluations(EA& ea) : _ea(ea), _df("fitness_evaluations.dat"), _evals(0) {
                _connections.resize(get<META_POPULATION_SIZE>(ea));
                
                for(std::size_t i=0; i<get<META_POPULATION_SIZE>(ea); ++i) {
                    _connections[i] = ea[i].events().fitness_evaluated.connect(boost::bind(&meta_population_fitness_evaluations::operator(), this, _1, _2));
                }
                
                _df.add_field("evaluation")
                .add_field("mean_generation")
                .add_field("min_fitness")
                .add_field("mean_fitness")
                .add_field("max_fitness");
            }
            
            virtual ~meta_population_fitness_evaluations() {
            }
            
            virtual void operator()(typename EA::individual_type::individual_type& ind, typename EA::individual_type& ea) {
                // record stats every 1000 fitness evaluations
                if((++_evals % 1000) == 0) {
                    using namespace boost::accumulators;
                    
                    accumulator_set<double, stats<tag::mean> > gen;
                    accumulator_set<double, stats<tag::min, tag::mean, tag::max> > fit;
                    
                    // iterating over the entire meta-population:
                    for(typename EA::iterator i=_ea.begin(); i!=_ea.end(); ++i) {
                        for(typename EA::individual_type::iterator j=i->begin(); j!=i->end(); ++j) {
                            if(ealib::has_fitness(*j,*i)) {
                                gen(j->generation());
                                fit(static_cast<double>(ealib::fitness(*j,*i)));
                            }
                        }
                    }
                    
                    _df.write(_evals)
                    .write(mean(gen))
                    .write(min(fit))
                    .write(mean(fit))
                    .write(max(fit))
                    .endl();
                }
            }
            
            EA& _ea;
            std::vector<boost::signals::scoped_connection> _connections;
            datafile _df;
            long _evals;
        };
        
    } // datafiles
} // ea

#endif
