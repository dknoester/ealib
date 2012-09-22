/* alps.h 
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
#ifndef _EA_ALPS_H_
#define _EA_ALPS_H_


#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <vector>
#include <limits>
#include <cmath>
#include <ea/events.h>
#include <ea/meta_data.h>

// update age - birth time
//     age = cur update - birth time -- a potential optimization.
// "reboot" lowest population
// parent selection scheme

namespace ea {
    // ea.alps*
    
    LIBEA_MD_DECL(GM_AGE, "ea.alps.genetic_material_age", unsigned int);
    LIBEA_MD_DECL(ADMISSION_AGE, "ea.alps.admission_age", double);
    LIBEA_MD_DECL(ALPS_MIN_REMAIN, "ea.alps.min_remain", double);
    
    template <typename EA>
    struct alps {
        
        alps(EA& ea) {
            _inheritance_conn = ea.events().inheritance.connect(boost::bind(&alps::inheritance, this, _1, _2, _3));
            _update_conn = ea.events().end_of_update.connect(boost::bind(&alps::end_of_update, this, _1));
            
            int number_levels = get<META_POPULATION_SIZE>(ea);
            for(int i=0; i<number_levels; ++i) {
                int age = i*10; 
                put<ADMISSION_AGE>(age, ea[i]);
            }
            
        }
        
        /*! Called for every inheritance event. 
         */
        virtual void inheritance(typename EA::population_type& parents,
                                 typename EA::individual_type& offspring,
                                 EA& ea) {
            int age =  ea.current_update(); 
            int p_age;
            for (typename EA::population_type::iterator i=parents.begin(); i!=parents.end(); ++i) {
                p_age = get<GM_AGE>(**i,0); 
                if (p_age > age) age = p_age + 1;
            }
            
            put<GM_AGE>(age, offspring);
        }
        
        //! Perform alps migration among populations.
        virtual void end_of_update(EA& ea) {
            int number_levels = get<META_POPULATION_SIZE>(ea);
            
            // increment the age of all individuals
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                for(typename EA::individual_type::population_type::iterator j=i->population().begin(); j!=i->population().end(); ++j) {
                    int age = get<GM_AGE>(**j) + 1; 
                    put<GM_AGE>(age, **j); 
                }
            }
            
            // "moving" an individual is insert/erase between populations
            // "copying" an individual is insert(make_population_entry(ind))
            for (int i = 0; i < (number_levels - 1); ++i) {
                double next_admission = get<ADMISSION_AGE>(ea[i+1]);
                typename EA::population_type moving_out;
                
                // sort ascending by fitness:
                std::sort(ea[i].population().begin(), ea[i].population().end(), comparators::meta_data<GM_AGE>());
                
                // find the first individual w/ fitness >= next_admission, but make sure to leave some behind
                typename EA::individual_type::population_type::iterator f=ea[i].population().begin();
                std::advance(f, static_cast<std::size_t>(get<ALPS_MIN_REMAIN>(ea)*get<POPULATION_SIZE>(ea)));
                typename EA::individual_type::population_type::iterator l=ea[i].population().end();
                for( ; f!=l; ++f) {
                    if (get<GM_AGE>(**f) > next_admission) {
                        break;
                    }
                }
                
                // now, move all individuals w/ fitness >= next_admission to the next pop:
                ea[i+1].population().insert(ea[i+1].population().end(), f,l);
                // and remove them from this one:
                ea[i].population().erase(f,l);
            }
        }
        
        boost::signals::scoped_connection _inheritance_conn;
        boost::signals::scoped_connection _update_conn;
    };
    
} // ea

#endif
