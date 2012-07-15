/* adaptive_hfc.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Randal S. Olson.
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

#ifndef _EA_ADAPTIVE_HFC_H_
#define _EA_ADAPTIVE_HFC_H_

#include <ea/events.h>

namespace ea {
    
    /*! Adaptive HFC migration among populations in a meta-population EA.
     */
    template <typename EA>
    struct adaptive_hfc : end_of_update_event<EA> {
        //! Constructor.
        adaptive_hfc(EA& ea) : end_of_update_event<EA>(ea) {
        }
        
        //! Destructor.
        virtual ~adaptive_hfc() {
        }
        
        //! Perform A-HFC migration among populations.
        virtual void operator()(EA& ea) {
            
            // "moving" an individual is insert/erase between populations
            // "copying" an individual is insert(make_population_entry(ind))
        }
    };

} // ea

#endif
