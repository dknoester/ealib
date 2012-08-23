/* digital_evolution/events.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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

#ifndef _EA_DIGITAL_EVOLUTION_EVENTS_H_
#define _EA_DIGITAL_EVOLUTION_EVENTS_H_

#include <ea/events.h>

namespace ea {    
    
    /*! Alife event handler.
     */
    template <typename EA>
	struct alife_event_handler : event_handler<EA> {
        //! Called when an individual performs a task.
        boost::signal<void(typename EA::individual_type&, // individual
                           double, // amount of resource consumed
                           const std::string&, // task name
                           EA&)> task_performed;
        
        //! Called when an individual is "born" (immediately after it is placed in the population).
        boost::signal<void(typename EA::individual_type&, // individual
                           EA&)> birth;
        
        //! Called when an individual "dies" or is replaced.
        boost::signal<void(typename EA::individual_type&, // individual
                           EA&)> death;
    };

} // ea

#endif
