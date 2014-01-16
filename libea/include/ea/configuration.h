/* configuration.h
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
#ifndef _EA_CONFIGURATION_H_
#define _EA_CONFIGURATION_H_

#include <ea/ptr_population.h>
#include <ea/ancestors.h>

namespace ealib {
     
    /*! Placeholder for user-defined configuration functions.
     */
    struct default_configuration {
        //! Called after construction of the EA.
        template <typename EA>
        void after_construction(EA& ea) {
        }
        
        //! Called after EA initialization.
        template <typename EA>
        void initialize(EA& ea) {
        }
        
        //! Called after the initial population has been generated.
        template <typename EA>
        void initial_population(EA& ea) {
        }
        
        //! Called to reset the state of this population (usually during meta-population experiments).
        template <typename EA>
        void reset(EA& ea) {
        }
    };
    
} // ealib

#endif
