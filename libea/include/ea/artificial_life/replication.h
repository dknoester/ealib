/* artificial_life/artificial_life.h 
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

#ifndef _EA_REPLICATION_H_
#define _EA_REPLICATION_H_

#include <boost/serialization/nvp.hpp>
#include <boost/tuple/tuple.hpp>
#include <ea/events.h>
#include <ea/interface.h>

namespace ea {
    
    /*! Selects the location of the first neighbor to the parent as the location
     for an offspring.
     
     This works well when combined with the well_mixed topology.  In this case, 
     the net effect is ~mass action.
     */
    struct first_neighbor {
        template <typename EA>
        std::pair<typename EA::environment_type::iterator, bool> operator()(typename EA::individual_ptr_type& parent, EA& ea) {
            return std::make_pair(ea.env().neighborhood(parent,ea).first, true);
        }
    };
    
    
    /*! (Re-)Place an offspring in the population, if possible.
     */
    template <typename EA>
    void replace(typename EA::individual_ptr_type parent, typename EA::individual_ptr_type offspring, EA& ea) {
        typename EA::replacement_type r;
        std::pair<typename EA::environment_type::iterator, bool> l=r(parent, ea);
            
        if(l.second) {            
            ea.env().replace(l.first, offspring, ea);
            offspring->priority() = parent->priority();
            ea.population().append(offspring);
            ea.events().birth(*offspring,ea);
        }
    }
    
    
    /*! Replicates a parent p to produce an offspring with representation r.
     */
    template <typename EA>
    void replicate(typename EA::individual_ptr_type p, typename EA::representation_type& r, EA& ea) {
        typename EA::population_type parents, offspring;
        parents.append(p);
        offspring.append(make_population_entry(typename EA::individual_type(r),ea));
        
        mutate(offspring.begin(), offspring.end(), ea);
        inherits(parents, offspring, ea);
        
        // parent is always reprioritized...
        ea.tasklib().prioritize(*p,ea);
        
        replace(*parents.begin(), *offspring.begin(), ea);
    }
    
    
    /*! This group replication method fills the offspring group with copies of a
     single mutated individual from the parent group.
     
     This works best when groups are assumed to be genetically homogeneous.
     */
    template <typename EA>
    void germline_replication(typename EA::individual_type& parent, typename EA::individual_type& offspring, EA& ea) {
        // grab a copy of the first individual:
        typename EA::individual_type::individual_type germ(**parent.population().begin());
        
        // mutate it:
        mutate(germ, offspring);
        
        // and now fill up the offspring population with copies of the germ:
        for(std::size_t j=0; j<get<POPULATION_SIZE>(offspring); ++j) {
            offspring.append(offspring.make_individual(germ.repr()));
        }        
    } 
    
} // ea

#endif
