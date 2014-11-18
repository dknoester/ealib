/* population_fitness.h
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
#ifndef _EA_DATAFILES_POPULATION_FITNESS_H_
#define _EA_DATAFILES_POPULATION_FITNESS_H_

#include <ea/datafile.h>
#include <ea/metadata.h>
#include <ea/traits.h>


namespace ealib {
    namespace datafiles {
        
        /*! Datafile for mean generation and min, mean, and max fitness.
         */
        template <typename EA>
        struct population_fitness_dat : record_statistics_event<EA> {
            population_fitness_dat(EA& ea) : record_statistics_event<EA>(ea), _df("population_fitness.dat") {
                _df.add_field("update")
                .add_field("individual")
                .add_field("fitness");
            }
            
            virtual ~population_fitness_dat() {
            }
            
            virtual void operator()(EA& ea) {
                for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                    _df.write(ea.current_update())
                    .write(get<IND_UNIQUE_NAME>(*i))
                    .write(static_cast<double>(ealib::fitness(*i,ea)))
                    .endl();
                }
            }
            
            datafile _df;
        };
        
    } // datafiles
} // ealib

#endif
