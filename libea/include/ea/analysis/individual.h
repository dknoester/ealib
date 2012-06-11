/* individual.h 
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

#ifndef _EA_ANALYSIS_MAX_FIT_INDIVIDUAL_H_
#define _EA_ANALYSIS_MAX_FIT_INDIVIDUAL_H_

#include <iostream>
#include <ea/meta_data.h>
#include <ea/analysis/tool.h>

namespace ea {
    namespace analysis {

        /*! Retest an individual.
         */
        template <typename EA>
        void test_individual(EA& ea) {
            typename EA::individual_type indi=individual_load(get<ANALYSIS_INPUT>(ea), ea);
            recalculate_fitness(indi,ea);
            individual_save(datafile(get<ANALYSIS_OUTPUT>(ea)), indi, ea);
        }
        // LIBEA_ANALYSIS_TOOL(test_individual, "test an individual")
        
        
        template <typename EA>
        typename EA::individual_type& find_most_fit_individual(EA& ea) {
            typename EA::population_type& pop = ea.population();
            typename EA::population_type::iterator mi=pop.begin();            
            for(typename EA::population_type::iterator i=pop.begin(); i!=pop.end(); ++i) {
                if(ind(i,ea).fitness() > ind(mi,ea).fitness()) {
                    mi = i;
                }
            }
            return ind(mi,ea);
        }
        
        
        /*! Retrieve an individual with the greatest fitness.
         */
        template <typename EA>
        void most_fit_individual(EA& ea) {
            individual_save(datafile(get<ANALYSIS_OUTPUT>(ea)), find_most_fit_individual(ea), ea);
        }        
        // LIBEA_ANALYSIS_TOOL(most_fit_individual, "select an individual with maximal fitness")

    } // analysis
} // ea

#endif
