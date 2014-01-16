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

#ifndef _EA_WELL_MIXED_H_
#define _EA_WELL_MIXED_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/serialization/nvp.hpp>
#include <utility>
#include <vector>
#include <ea/meta_data.h>

namespace ealib {
    
    /*! Well-mixed topology.
     */
    template <typename EA>
    struct well_mixed {
        typedef EA ea_type; //<! EA type using this topology.        
        typedef typename ea_type::individual_ptr_type individual_ptr_type; //!< Pointer to individual type.
                
        /*! Location type.
         
         While locations logically "live" inside organisms, they are interpreted 
         by the specific topology being used.  So, the topology "owns" the various
         locations, but organisms have pointers to the specific location at which
         they reside.
         
         Locations also have a pointer to their organism, thus we can go betwixt
         them with ease.
         */
        struct location_type {
            //! Location meta-data.
            meta_data& md() { return _md; }
            
            //! Is this location occupied?
            bool occupied() { return p != 0; }
            
            //! Return the inhabitant.
            individual_ptr_type inhabitant() { return p; }
            
            individual_ptr_type p; //!< Individual (if any) at this location.
            meta_data _md; //!< Meta-data container.
        };
        
        //! Location type pointer (lives in organism).
        typedef location_type* location_ptr_type;

        typedef std::vector<location_type> location_list_type; //!< Container type for locations.

        /*! Well-mixed neighborhood iterator.
         
         The idea here is that the underlying topology of a well-mixed environment is random.  This
         iterator class thus provides a random (w/ replacement) sequence of locations.
         
         The "end" iterator is really just a a number of dereferences, set to the number of possible
         locations.  Note that this is *not* the same as iterating over all locations.
         */
        struct iterator : boost::iterator_facade<iterator, location_type, boost::single_pass_traversal_tag> {
            //! Constructor.
            iterator(std::size_t n, location_list_type& locs, ea_type& ea) : _n(n), _locs(locs), _ea(ea) {
            }
            
            //! Increment operator.
            void increment() { ++_n; }
            
            //! Iterator equality comparison.
            bool equal(iterator const& that) const { return _n == that._n; }
            
            //! Dereference this iterator.
            location_type& dereference() const { return *_ea.rng().choice(_locs.begin(), _locs.end()); }
            
            std::size_t _n; //!< how many times this iterator has been dereferenced.
            location_list_type& _locs; //!< list of all possible locations.
            ea_type& _ea; //!< EA (used for rngs, primarily).
        };                

        //! Constructor.
        well_mixed() : _occupied(0) {
        }
        
        //! Initialize this topology.
        void initialize(ea_type& ea) {
            _locs.resize(get<POPULATION_SIZE>(ea));
        }
        
        //!< Retrieve the neighborhood of the given individual.
        std::pair<iterator,iterator> neighborhood(individual_ptr_type p, ea_type& ea) {
            return std::make_pair(iterator(0,_locs,ea), iterator(_locs.size(),_locs,ea));
        }                
        
//        //! Retrieve the currently faced neighboring location of the given individual.
//        iterator neighbor(individual_ptr_type p, ea_type& ea) {
////            return iterator(*p->location(),p->location()->heading,_locs,ea);            
//        }

        //! Replace the organism (if any) living in location l with p.
        void replace(iterator i, individual_ptr_type p, ea_type& ea) {
            location_type& l=(*i);
            // kill the occupant of l, if any
            if(l.p) {
                l.p->alive() = false;
                ea.events().death(*l.p,ea);
            }
            l.p = p;
        }
                
        void insert(individual_ptr_type p) {
            _locs[_occupied++].p = p;
        }
        
        int size() { return _locs.size(); }
        
        //! Read from the environment.
        template <typename Organism>
        int read(Organism& org, ea_type& ea) {
            return ea.rng()(std::numeric_limits<int>::max());
        }
        
        //! Serialize this topology.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
        }
        
        int _occupied; //!< How many locations are currently occupied?
        location_list_type _locs; //!< List of all locations in this topology.
    };
    
} // ea

#endif
