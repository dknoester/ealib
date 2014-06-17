/* multiobjective_fitness.h
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
#ifndef _EA_DATAFILES_MULTIOBJECTIVE_FITNESS_H_
#define _EA_DATAFILES_MULTIOBJECTIVE_FITNESS_H_

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
        struct multiobjective_fitness_dat : record_statistics_event<EA> {
            multiobjective_fitness_dat(EA& ea) : record_statistics_event<EA>(ea), _df("multiobjective_fitness.dat") {
                _df.add_field("update")
                .add_field("mean_generation");
                
                for(std::size_t i=0; i<ea.fitness_function().size(); ++i) {
                    std::ostringstream f;
                    f << "max_fitness_" << i;
                    _df.add_field(f.str());
                }
            }
            
            virtual ~multiobjective_fitness_dat() {
            }
            
            virtual void operator()(EA& ea) {
                std::size_t fsize = ea.fitness_function().size();
                using namespace boost::accumulators;
                accumulator_set<double, stats<tag::mean> > gen;
                std::vector<accumulator_set<double, stats<tag::max> > > fit(fsize);
                
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    gen(get<IND_GENERATION>(*i));
                    typename EA::fitness_type& f=ealib::fitness(*i,ea);
                    for(std::size_t j=0; j<fsize; ++j) {
                        fit[j](static_cast<double>(f[j]));
                    }
                }
                
                _df.write(ea.current_update())
                .write(mean(gen));
                
                for(std::size_t i=0; i<fsize; ++i) {
                    _df.write(max(fit[i]));
                }
                _df.endl();
            }
            
            datafile _df;
        };
        
    } // datafiles
} // ealib

#endif
