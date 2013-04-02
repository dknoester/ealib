/* generation_fitness.h
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
#ifndef _EA_digital_evolution_DATAFILES_REACTIONS_H_
#define _EA_digital_evolution_DATAFILES_REACTIONS_H_

#include <ea/datafile.h>
#include <ea/events.h>

namespace ealib {
    namespace datafiles {
        
        /*! Datafile for reactions.
         */
        template <typename EA>
        struct record_reactions_event : record_statistics_event<EA> {
            record_reactions_event(EA& ea) : record_statistics_event<EA>(ea), _df("reactions.dat") {
                _df.add_field("update")
                .add_field("not")
                .add_field("nand")
                .add_field("and")
                .add_field("ornot")
                .add_field("or")
                .add_field("andnot")
                .add_field("nor")
                .add_field("xor")
                .add_field("equals");
                
                _conn2 = ea.events().task_performed.connect(boost::bind(&record_reactions_event::record_task, this, _1, _2, _3, _4));
            }
            
            virtual ~record_reactions_event() {
            }
            
            
            void record_task(typename EA::individual_type& ind, // individual
                             double r, // amount of resource consumed
                             const std::string& task, // task name
                             EA& ea) {
                _tasks[task] += r;
            }
            
            virtual void operator()(EA& ea) {
                _df.write(ea.current_update())
                .write(_tasks["not"])
                .write(_tasks["nand"])
                .write(_tasks["and"])
                .write(_tasks["ornot"])
                .write(_tasks["or"])
                .write(_tasks["andnot"])
                .write(_tasks["nor"])
                .write(_tasks["xor"])
                .write(_tasks["equals"])
                .endl();
                _tasks.clear();
            }
            
            boost::signals::scoped_connection _conn2;
            datafile _df;
            std::map<std::string, unsigned int> _tasks;
        };

    } // datafiles
} // ea

#endif
