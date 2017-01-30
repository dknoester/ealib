/* digital_evolution/replication.h 
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

#ifndef _EA_DIGITAL_EVOLUTION_REPLICATION_H_
#define _EA_DIGITAL_EVOLUTION_REPLICATION_H_


namespace ealib {
    

    /*! Selects the location of the first neighbor to the parent as the location
     for an offspring.
     */
    struct first_neighbor {
        template <typename EA>
        std::pair<typename EA::location_iterator, bool> operator()(typename EA::individual_ptr_type parent, EA& ea) {
            return std::make_pair(ea.env().neighborhood(parent,ea).first.make_location_iterator(), true);
        }
    };
    
    /*! Selects the location of a random neighbor to the parent as the location
     for an offspring.
     */
    struct random_neighbor {
        template <typename EA>
        std::pair<typename EA::location_iterator, bool> operator()(typename EA::individual_ptr_type parent, EA& ea) {
            std::pair<typename EA::neighborhood_iterator, typename EA::neighborhood_iterator> i = ea.env().neighborhood(*parent);
            typename EA::location_iterator l = ea.rng().choice(i.first, i.second).make_location_iterator();
            assert((l->r[0] != parent->position().r[0]) || (l->r[1] != parent->position().r[1]));
            return std::make_pair(l, true);
        }
    };

    /*! Selects the location of the neighbor faced by the parent as the location
     for an offspring.
     */
    struct empty_facing_neighbor {
        template <typename EA>
        std::pair<typename EA::location_iterator, bool> operator()(typename EA::individual_ptr_type parent, EA& ea) {
            typename EA::location_iterator l = ea.env().neighbor(parent);
            if (l->occupied()) {
                return std::make_pair(l, false);

            }
            return std::make_pair(l, true);
        }
    };
    
    /*! Selects the location of the neighbor faced by the parent (using a matrix) as the location
     for an offspring.
     */
    struct empty_facing_neighbor_matrix {
        template <typename EA>
        std::pair<typename EA::location_iterator, bool> operator()(typename EA::individual_ptr_type parent, EA& ea) {
            typename EA::location_iterator l = ea.env().neighbor(parent);
            typename EA::environment_type::location_type mp = ea.env().location(parent->position());

            // check to make sure we aren't peeking around an edge
            int me_x = mp.r[0];
            int me_y = mp.r[1];
            
            int max_x = get<SPATIAL_X>(ea) - 1;
            int max_y = get<SPATIAL_Y>(ea) - 1;
            
            int you_x = l->r[0];
            int you_y = l->r[1];
            
            // If the neighbor wraps around, exit.
            if (((me_x == 0 ) && (you_x == max_x)) ||
                ((me_x == max_x) && (you_x == 0)) ||
                ((me_y == 0) && (you_y == max_y)) ||
                ((me_y == max_y) && (you_y == 0))) {
                return std::make_pair(l, false);
            }

            
            if (l->occupied()) {
                return std::make_pair(l, false);
                
            }
            return std::make_pair(l, true);
        }
    };
    
    /*! Selects the location of an empty neighbor location to the parent as the location
     for an offspring. (Note: here empty includes locations occupied by dead organisms.)
     */
    struct empty_neighbor {
        template <typename EA>
        std::pair<typename EA::location_iterator, bool> operator()(typename EA::individual_ptr_type parent, EA& ea) {
            std::pair<typename EA::neighborhood_iterator, typename EA::neighborhood_iterator> i = ea.env().neighborhood(*parent);
            for( ; i.first != i.second; ++i.first) {
                if(!i.first->occupied()) {
                    return std::make_pair(i.first.make_location_iterator(), true);
                }
            }
            return std::make_pair(i.second.make_location_iterator(), false);
        }
    };
    
    /*! Replicates a parent p to produce an offspring with representation r.
     */
    template <typename EA>
    void replicate(typename EA::individual_ptr_type p, typename EA::genome_type& r, EA& ea) {
        typename EA::population_type parents, offspring;
        parents.push_back(p);
        offspring.push_back(ea.make_individual(r));
        
        mutate(offspring.begin(), offspring.end(), ea);
        inherits(parents, offspring, ea);
        
        // parent is always reprioritized...
        ea.tasklib().prioritize(*p,ea);
        
        // this handles prioritizing the offspring:
        ea.replace(*parents.begin(), *offspring.begin());
    }

} // ea

#endif
