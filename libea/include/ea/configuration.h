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

#include <ea/events.h>

namespace ea {
    
    // pre-dec
    template <typename EA> class abstract_configuration;
    
    //! Add an event to the list of events that are registered for an EA.
    template <template <typename> class Event, typename EA>
    void add_event(abstract_configuration<EA>* config, EA& ea) {
        typedef Event<EA> event_type;
        boost::shared_ptr<event_type> p(new event_type(ea));
        config->_events.push_back(p);
    }
    

    
    /*! Abstract configuration object for an EA.
     
     This struct defines the high-level life cycle events of an EA that are expected
     to be user-configurable.  These methods are not called frequently, but rather
     are designed to intercept control flow at those points that commonly require
     experiment-specific changes.
     */
    template <typename EA>
    class abstract_configuration {
    public:
        typedef std::vector<boost::shared_ptr<ea::event> > event_list; //!< Storage for events.
        
        //! Called as the final step of EA construction.
        virtual void construct(EA& ea) {
        }
        
        //! Called as the final step of EA initialization.
        virtual void initialize(EA& ea) {
        }

        //! Called to reset the state of this population (usually during meta-population experiments).
        virtual void reset(EA& ea) {
        }

        //! Called to generate the initial EA population.
        virtual void initial_population(EA& ea) {
        }
    protected:
        template <template <typename> class T, typename U> friend void add_event(abstract_configuration<U>* config, U& u);
        event_list _events; //!< List of all the events attached to an EA.

    };

}
#endif
