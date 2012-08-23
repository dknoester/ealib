/* digital_evolution/digital_evolution.h 
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

#ifndef _EA_DIGITAL_EVOLUTION_REPLICATION_H_
#define _EA_DIGITAL_EVOLUTION_REPLICATION_H_


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

    
    /*! Replicates a parent p to produce an offspring with representation r.
     */
    template <typename EA>
    void replicate(typename EA::individual_ptr_type p, typename EA::representation_type& r, EA& ea) {
        typename EA::population_type parents, offspring;
        parents.push_back(p);
        offspring.push_back(ea.make_individual(r));
        
        mutate(offspring.begin(), offspring.end(), ea);
        inherits(parents, offspring, ea);
        
        // parent is always reprioritized...
        ea.tasklib().prioritize(*p,ea);
        
        ea.replace(*parents.begin(), *offspring.begin());
    }

} // ea

#endif
