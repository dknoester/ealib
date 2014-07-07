/* reactions.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2014 David B. Knoester.
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
#ifndef _EA_DATAFILES_REACTIONS_H_
#define _EA_DATAFILES_REACTIONS_H_

#include <ea/datafile.h>
#include <ea/events.h>

namespace ealib {
    namespace datafiles {
        
        /*! Datafile for reactions.
         */
        template <typename EA>
        struct reactions : record_statistics_event<EA> {
            reactions(EA& ea) : record_statistics_event<EA>(ea), _df("reactions.dat") {
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
                
                _conn2 = ea.events().reaction.connect(boost::bind(&reactions::on_task, this, _1, _2, _3, _4));
            }
            
            virtual ~reactions() {
            }
            
            void on_task(typename EA::individual_type& ind, // individual
                         typename EA::task_library_type::task_ptr_type t, // task pointer
                         double r, // resources consumed
                         EA& ea) {
                _tasks[t->name()] += r;
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
            
            datafile _df;
            boost::signals2::scoped_connection _conn2;
            std::map<std::string, double> _tasks;
        };

    } // datafiles
} // ea

#endif
