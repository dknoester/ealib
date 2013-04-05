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
#include <ea/attributes.h>
#include <ea/meta_data.h>
#include <ea/analysis/tool.h>

namespace ealib {
    namespace analysis {

        template <typename EA>
        typename EA::individual_type& find_dominant(EA& ea) {
            typename EA::population_type& pop = ea.population();
            typename EA::population_type::iterator mi=pop.begin();
            for(typename EA::population_type::iterator i=pop.begin(); i!=pop.end(); ++i) {
                if(ealib::fitness(**i,ea) > ealib::fitness(**mi,ea)) {
                    mi = i;
                }
            }
            return **mi;
        }

    } // analysis
} // ea

#endif
