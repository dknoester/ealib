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
                       typename EA::tasklib_type::task_ptr_type, // task pointer
                       double r, // resources consumed
                       EA&)> task_performed;
    
    //! Called when an individual is "born" (immediately after it is placed in the population).
    boost::signal<void(typename EA::individual_type&, // individual offspring
                       typename EA::individual_type&, // individual parent
                       EA&)> birth;
    
    //! Called when an individual "dies" or is replaced.
    boost::signal<void(typename EA::individual_type&, // individual
                       EA&)> death;
  };
  
  
  template <typename EA>
  struct task_performed_event : event {
    task_performed_event(EA& ea) {
      conn = ea.events().task_performed.connect(boost::bind(&task_performed_event::operator(), this, _1, _2, _3, _4));
    }
    virtual ~task_performed_event() { }
    virtual void operator()(typename EA::individual_type&, // individual
                            typename EA::tasklib_type::task_ptr_type, // task pointer
                            double r, // resources consumed
                            EA&) = 0;
  };
  
  template <typename EA>
  struct birth_event : event {
    birth_event(EA& ea) {
      conn = ea.events().birth.connect(boost::bind(&birth_event::operator(), this, _1, _2, _3));
    }
    virtual ~birth_event() { }
    virtual void operator()(typename EA::individual_type&, // individual offspring
                            typename EA::individual_type&, // individual parent
                            EA&) = 0;
  };
  
  template <typename EA>
  struct death_event : event {
    death_event(EA& ea) {
      conn = ea.events().death.connect(boost::bind(&death_event::operator(), this, _1, _2));
    }
    virtual ~death_event() { }
    virtual void operator()(typename EA::individual_type&, // individual
                            EA&) = 0;
  };
  
} // ea

#endif
