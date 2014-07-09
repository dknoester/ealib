/* digital_evolution/events.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester, Heather J. Goldsby.
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

namespace ealib {
    
    /*! Digital evolution event handler.
     */
    template <typename EA>
	class digital_evolution_event_handler : public event_handler<EA> {
    public:
        //! Default constructor.
        digital_evolution_event_handler() {
        }
        
        //! Called when an individual performs a task.
        boost::signals2::signal<void(typename EA::individual_type&, // individual
                                     typename EA::task_library_type::task_ptr_type, // task pointer
                                     EA&)> task;
        
        //! Called when an individual participates in a reaction.
        boost::signals2::signal<void(typename EA::individual_type&, // individual
                                     typename EA::task_library_type::task_ptr_type, // task pointer
                                     double r, // resources consumed
                                     EA&)> reaction;
        
        //! Called when an individual is "born" (immediately after it is placed in the population).
        boost::signals2::signal<void(typename EA::individual_type&, // individual offspring
                                     typename EA::individual_type&, // individual parent
                                     EA&)> birth;
        
        //! Called when an individual "dies" or is replaced.
        boost::signals2::signal<void(typename EA::individual_type&, // individual
                                     EA&)> death;
        
    private:
        digital_evolution_event_handler(const digital_evolution_event_handler&);
        digital_evolution_event_handler& operator=(const digital_evolution_event_handler&);
    };
    
    
    template <typename EA>
    struct task_event : event {
        task_event(EA& ea) {
            conn = ea.events().task.connect(boost::bind(&task_event::operator(), this, _1, _2, _3));
        }
        virtual ~task_event() { }
        virtual void operator()(typename EA::individual_type&, // individual
                                typename EA::task_library_type::task_ptr_type, // task pointer
                                EA&) = 0;
    };
    
    template <typename EA>
    struct reaction_event : event {
        reaction_event(EA& ea) {
            conn = ea.events().reaction.connect(boost::bind(&reaction_event::operator(), this, _1, _2, _3, _4));
        }
        virtual ~reaction_event() { }
        virtual void operator()(typename EA::individual_type&, // individual
                                typename EA::task_library_type::task_ptr_type, // task pointer
                                double, // resources consumed
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
