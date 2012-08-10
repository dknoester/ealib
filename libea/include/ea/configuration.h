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

namespace ea {
    
    /*! Abstract configuration object for an EA.
     
     This struct defines the high-level life cycle events of an EA that are expected
     to be user-configurable.  These methods are not called frequently, but rather
     are designed to intercept control flow at those points that commonly require
     experiment-specific changes.
     */
    template <typename EA>
    struct abstract_configuration {
        
        //! Called as the final step of EA construction.
        virtual void construct(EA& ea) {
        }
        
        //! Called as the final step of EA initialization.
        virtual void initialize(EA& ea) {
        }
        
        //! Called to generate the initial EA population.
        virtual void initial_population(EA& ea) {
        }
        
    };

}
#endif
