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
#ifndef _EA_ISLAND_MODEL_H_
#define _EA_ISLAND_MODEL_H_


#include <ea/events.h>
#include <ea/meta_data.h>

namespace ea {
    LIBEA_MD_DECL(ADMISSION_LEVEL, "ea.adaptive_hfc.admission_level", double);

    /*! Island models provide for migration among different populations in a 
     meta-population EA.
     */
    template <typename EA>
    struct island_model : end_of_update_event<EA> {
        //! Constructor.
        island_model(EA& ea) : end_of_update_event<EA>(ea) {
        }
        
        //! Destructor.
        virtual ~island_model() {
        }
        
        //! Perform migration of individuals among populations.
        virtual void operator()(EA& ea) {
        }        
    };
    
} // ea

#endif
