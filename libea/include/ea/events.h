/* events.h
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
#ifndef _EA_EVENTS_H_
#define _EA_EVENTS_H_

#include <boost/bind.hpp>
#include <boost/signal.hpp>

#include <ea/interface.h>

namespace ea {

    /*! Contains event handlers for generic events of interest within an evolutionary
	 algorithm.
	 
     An easy way to attach to any of these events is by subclassing the *_event structs
     below.  Because they use a boost::bind expression to connect to the signals, we
     avoid any difficulty with copy construction of the slots.
	 */
	template <typename EA>
	struct event_handler {
		
        /*! Called after the fitness of an individual has been evaluated.
         */
        boost::signal<void(typename EA::individual_type&, // individual
                           EA&)> fitness_evaluated;
        
		/*! Called at the end of every update.
		 */
		boost::signal<void(EA&)> end_of_update;
		
		/*! Called after every epoch.
		 */
		boost::signal<void(EA&)> end_of_epoch;
		
		/*! Called when an offspring individual inherits from its parents.
		 */
		boost::signal<void(typename EA::population_type&, // parents
                           typename EA::individual_type&, // offspring
                           EA&)> inheritance;

        /*! Called when an individual asexually replicates.
		 */
		boost::signal<void(typename EA::individual_type&, // parent
                           typename EA::individual_type&, // offspring
                           EA&)> replication;

        /*! Called at the beginning of epochs and at the end of every generation.
		 */
		boost::signal<void(EA&)> record_statistics;
    };

    
    /*! Base class for different events.
     */
    struct event {
        virtual ~event() {
        }        
        boost::signals::scoped_connection conn;
    };
    
    
    template <typename EA>
    struct fitness_evaluated_event : event {
        fitness_evaluated_event(EA& ea) {
            conn = ea.events().fitness_evaluated.connect(boost::bind(&fitness_evaluated_event::operator(), this, _1, _2));
        }
        virtual ~fitness_evaluated_event() { }
        virtual void operator()(typename EA::individual_type& ind, EA& ea) = 0;
    };
    
    template <typename EA>
    struct end_of_update_event : event {
        end_of_update_event(EA& ea) {
            conn = ea.events().end_of_update.connect(boost::bind(&end_of_update_event::operator(), this, _1));
        }
        virtual ~end_of_update_event() { }
        virtual void operator()(EA& ea) = 0;
    };
    
    template <typename MDType, typename EA>
    struct periodic_event : event {
        periodic_event(EA& ea) : _n(0) {
            conn = ea.events().end_of_update.connect(boost::bind(&periodic_event::end_of_update, this, _1));
        }
        virtual ~periodic_event() { }
        
        virtual void end_of_update(EA& ea) {
            unsigned long g = current_update(ea);
            if((g % static_cast<unsigned long>(get<MDType>(ea))) == 0) {
                operator()(ea);
            }
        }

        virtual void operator()(EA& ea) = 0;
        
        unsigned int period() const { return _n; }
        
        unsigned int _n; //!< Current period.
    };
    
    template <typename EA>
    struct end_of_epoch_event : event {
        end_of_epoch_event(EA& ea) {
            conn = ea.events().end_of_epoch.connect(boost::bind(&end_of_epoch_event::operator(), this, _1));
        }
        virtual ~end_of_epoch_event() { }
        virtual void operator()(EA& ea) = 0;
    };
    
    template <typename EA>
    struct record_statistics_event : event {
        record_statistics_event(EA& ea) {
            conn = ea.events().record_statistics.connect(boost::bind(&record_statistics_event::record, this, _1));
        }
        virtual ~record_statistics_event() { }
        virtual void record(EA& ea) {
            if((ea.current_update() == 0) || ((ea.current_update() % get<RECORDING_PERIOD>(ea)) == 0)) {
                operator()(ea);
            }
        }
        virtual void operator()(EA& ea) = 0;
    };

    template <typename EA>
    struct inheritance_event : event {
        inheritance_event(EA& ea) {
            conn = ea.events().inheritance.connect(boost::bind(&inheritance_event::operator(), this, _1, _2, _3));
        }
        virtual ~inheritance_event() { }
        virtual void operator()(typename EA::population_type&, // parents
                                typename EA::individual_type&, // offspring
                                EA&) = 0;
    };
    
    template <typename EA>
    struct replication_event : event {
        replication_event(EA& ea) {
            conn = ea.events().replication.connect(boost::bind(&replication_event::operator(), this, _1, _2, _3));
        }
        virtual ~replication_event() { }
        virtual void operator()(typename EA::individual_type&, // parent
                                typename EA::individual_type&, // offspring
                                EA&) = 0;
    };
    
} // ea

#endif
