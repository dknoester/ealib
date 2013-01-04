/* population.h 
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

#ifndef _EA_ANALYSIS_POPULATION_H_
#define _EA_ANALYSIS_POPULATION_H_

#include <ea/datafile.h>



namespace ea {
    namespace analysis {

        /*! Save the fitness values of all individuals in the population.
         */
        template <typename EA>
        struct population_fitness : public ea::analysis::unary_function<EA> {
            static const char* name() { return "population_fitness"; }
            
            virtual void operator()(EA& ea) {
                
                datafile df(get<ANALYSIS_OUTPUT>(ea));
                for(typename EA::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
                    df.write(ind(i,ea).fitness());
                }
                df.endl();
                
            }
        };
        
    } // analysis
} // ea

#endif
