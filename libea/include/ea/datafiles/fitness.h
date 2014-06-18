/* fitness.h
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
#ifndef _EA_DATAFILES_FITNESS_H_
#define _EA_DATAFILES_FITNESS_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <ea/datafile.h>
#include <ea/traits.h>


namespace ealib {
    namespace datafiles {
        
        /*! Datafile for mean generation and min, mean, and max fitness.
         */
        template <typename EA>
        struct fitness_dat : record_statistics_event<EA> {
            fitness_dat(EA& ea) : record_statistics_event<EA>(ea), _df("fitness.dat") {
                _df.add_field("update")
                .add_field("mean_generation")
                .add_field("min_fitness")
                .add_field("mean_fitness")
                .add_field("max_fitness");
            }
            
            virtual ~fitness_dat() {
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
 
    } // datafiles
} // ealib

#endif
