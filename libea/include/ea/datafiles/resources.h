/* resources.h
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
#ifndef _EA_DATAFILES_RESOURCES_H_
#define _EA_DATAFILES_RESOURCES_H_

#include <ea/datafile.h>
#include <ea/events.h>

namespace ealib {
    namespace datafiles {
        
        /*! Datafile for resource levels.
         */
        template <typename EA>
        struct resources : record_statistics_event<EA> {
            typedef typename EA::environment_type::resource_list_type resource_list_type;

            resources(EA& ea) : record_statistics_event<EA>(ea), _df("resources.dat") {
                resource_list_type& res = ea.env().resources();
                
                _df.add_field("update");
                for(typename resource_list_type::iterator i=res.begin(); i!=res.end(); ++i) {
                    _df.add_field((*i)->name());
                }
            }
            
            virtual ~resources() {
            }

            virtual void operator()(EA& ea) {
                resource_list_type& res = ea.env().resources();

                _df.write(ea.current_update());
                for(typename resource_list_type::iterator i=res.begin(); i!=res.end(); ++i) {
                    _df.write((*i)->level());
                }
                _df.endl();
            }
            
            datafile _df;
        };
        
    } // datafiles
} // ea

#endif
