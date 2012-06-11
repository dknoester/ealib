/* landscape.h
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
#ifndef _EA_LANDSCAPE_H_
#define _EA_LANDSCAPE_H_

#include <ea/interface.h>
#include <ea/meta_data.h>

namespace ea {
    namespace landscape {
        
        /*! This event periodically reinitializes the landscape for the entire 
         population.
         
         Note that this triggers a fitness reevaluation for all individuals in
         the population.
         
         \todo What this means is that each individual no longer experiences
         a single fitness landscape.  This is rather a-lifey, and has some 
         interesting implications for perfectly recreating an individual's 
         history for later analysis.
         
         For the moment, we're not going to worry about this.
         */
        template <typename EA>
        struct landscape_reinitialization : periodic_event<LANDSCAPE_PERIOD, EA> {
            landscape_reinitialization(EA& ea) : periodic_event<LANDSCAPE_PERIOD, EA>(ea) {
            }
            
            virtual void operator()(EA& ea) {
                ea.fitness_function().reinitialize(ea);
                recalculate_fitness(ea.population().begin(), ea.population().end(), ea);
            }
        };

    } // landscape
} // ea

#endif
