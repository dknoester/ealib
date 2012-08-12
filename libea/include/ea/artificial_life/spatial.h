/* artificial_life/spatial.h 
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

#ifndef _EA_SPATIAL_H_
#define _EA_SPATIAL_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <utility>
#include <vector>
#include <ea/algorithm.h>
#include <ea/meta_data.h>

namespace ea {
    
    struct empty { };
    
    LIBEA_MD_DECL(SPATIAL_X, "ea.environment.x", unsigned int);
    LIBEA_MD_DECL(SPATIAL_Y, "ea.environment.y", unsigned int);
    
    
    namespace resources {
        //! Abstract resource type.
        struct abstract_resource {
            abstract_resource(const std::string& name) : _name(name) { }
            //! Update resource levels, if needed, based on elapsed time since last update (as a fraction of update length).
            virtual void update(double delta_t) = 0;
            //! Consume resources.
            virtual double consume() = 0;
            
            std::string _name;
        };
        
        //! Unlimited resource type.
        struct unlimited : abstract_resource {
            unlimited(const std::string& name) : abstract_resource(name) { }
            void update(double) { }
            double consume() { return 1.0; }
        };
        
        //! Limited resource type.
        struct limited : abstract_resource {
            limited(const std::string& name, double initial, double inflow, double outflow, double consume)
            : abstract_resource(name), _level(initial), _inflow(inflow), _outflow(outflow), _consume(consume) {
            }
            
            void update(double delta_t) {
                _level += delta_t * (_inflow - _outflow * _level);
                _level = std::max(0.0, _level);
            }
            
            double consume() {
                double r = std::max(0.0, _level*_consume);
                _level = std::min(0.0, _level-r);
                return r;
            }

            double _level; //!< Current resource level.
            double _inflow; //!< Amount of resource flowing in per update.
            double _outflow; //!< Rate at which resource flows out per update.
            double _consume; //!< Fraction of resource consumed.
        };
        
    } // resources

    //! Helper method that builds an unlimited resource and adds it to the environment.
    template <typename EA>
    typename EA::environment_type::resource_ptr_type make_resource(const std::string& name, EA& ea) {
        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type;
        resource_ptr_type p(new resources::unlimited(name));
        ea.env().add_resource(p);
        return p;
    }
    
    //! Helper method that builds a limited resource and adds it to the environment.
    template <typename EA>
    typename EA::environment_type::resource_ptr_type make_resource(const std::string& name, double initial, double inflow, double outflow, double consume, EA& ea) {
        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type;
        resource_ptr_type p(new resources::limited(name, initial, inflow, outflow, consume));
        ea.env().add_resource(p);
        return p;
    }

    
    /*! Spatial topology.
     */
    template <typename EA>
    struct spatial {
        typedef EA ea_type; //<! EA type using this topology.        
        typedef typename ea_type::individual_ptr_type individual_ptr_type; //!< Pointer to individual type.
        typedef typename ea_type::individual_type individual_type; //!< Pointer to individual type.
        
        typedef boost::shared_ptr<resources::abstract_resource> resource_ptr_type;
        typedef std::vector<resource_ptr_type> resource_list_type;
        resource_list_type _resources;
        
        /*! Location type.
         
         While locations logically "live" inside organisms, they are interpreted 
         by the specific topology being used.  So, the topology "owns" the various
         locations, but organisms have pointers to the specific location at which
         they reside.
         
         Locations also have a pointer to their organism, thus we can go betwixt
         them with ease.
         */
        struct location_type {
            //! Constructor.
            location_type() : heading(0), x(0), y(0) {
            }
            
            //! Location meta-data.
            meta_data& md() { return _md; }

            //! Is this location occupied?
            bool occupied() { return ((p != 0) && (p->alive())); }
            
            //! Return the inhabitant.
            individual_ptr_type inhabitant() { return p; }
            
            //! Set the heading of this location.
            void set_heading(int h) {
                heading = h % 8;
            }
            
            //! Alter the heading of this location.
            void alter_heading(int h) {
                heading = algorithm::roll(heading+h, 0, 7);
            }
            
            individual_ptr_type p; //!< Individual (if any) at this location.
            int heading; //!< Heading of organism, in degrees.
            int x,y; //!< X-Y coordinates of this location.
            meta_data _md; //!< Meta-data container.
        };
        
        //! Location type pointer (lives in organism).
        typedef location_type* location_ptr_type;
                
        typedef boost::numeric::ublas::matrix<location_type> location_matrix_type; //! Container type for locations.
        
        /*! Spatial neighborhood iterator.
         */
        struct iterator : boost::iterator_facade<iterator, location_type, boost::single_pass_traversal_tag> {
            //! Constructor.
            iterator(location_type& origin, int h, location_matrix_type& locs, ea_type& ea) 
            : _origin(origin), _heading(h), _locs(locs), _ea(ea) {
            }
            
            //! Increment operator.
            void increment() { ++_heading; }
            
            //! Iterator equality comparison.
            bool equal(iterator const& that) const { 
                return (_origin.y==that._origin.y) && (_origin.x==that._origin.x) && (_heading==that._heading); 
            }
            
            /*! Dereference this iterator.
             
             (x,y) == (column,row)
             (0,0) == lower left
             
             Unit circle:
             3  |  2  |  1
             4  |  Or.|  0
             5  |  6  |  7
             */
            location_type& dereference() const { 
                int x=_origin.x;
                int y=_origin.y;
                
                switch(_heading%8) {
                    case 0: ++x; break;
                    case 1: ++x; ++y; break;
                    case 2: ++y; break;
                    case 3: --x; ++y; break;
                    case 4: --x; break;
                    case 5: --x; --y; break;
                    case 6: --y; break;
                    case 7: ++x; --y; break;
                }

                x = algorithm::roll(x, 0, static_cast<int>(_locs.size2()-1));
                y = algorithm::roll(y, 0, static_cast<int>(_locs.size1()-1));

                return _locs(y,x); // correct: y==i, x==j.
            }
            
            location_type& _origin; //!< Origin of this iterator.
            int _heading; //!< Current heading for the iterator.
            location_matrix_type& _locs; //!< list of all possible locations.
            ea_type& _ea; //!< EA (used for rngs, primarily).
        };                
        
        //! Constructor.
        spatial() : _occupied(0) {
        }
        
        //! Initialize this topology.
        void initialize(ea_type& ea) {
            _locs.resize(get<SPATIAL_Y>(ea), get<SPATIAL_X>(ea), false);
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    _locs(i,j).x = j;
                    _locs(i,j).y = i;
                }
            }
        }
        
        //! Retrieve the neighborhood of the given individual.
        std::pair<iterator,iterator> neighborhood(individual_ptr_type p, ea_type& ea) {
            return std::make_pair(iterator(*p->location(),0,_locs,ea),
                                  iterator(*p->location(),8,_locs,ea));
        }
        
        //! Retrieve the currently faced neighboring location of the given individual.
        iterator neighbor(individual_ptr_type p, ea_type& ea) {
            return iterator(*p->location(),p->location()->heading,_locs,ea);            
        }
        
        //! Replace the organism (if any) living in location l with p.
        void replace(iterator i, individual_ptr_type p, ea_type& ea) {
            location_type& l=(*i);
            // kill the occupant of l, if any
            if(l.p) {
                l.p->alive() = false;
                ea.events().death(*l.p,ea);
            }
            l.p = p;
            p->location() = &l;
        }
        
        void insert(individual_ptr_type p) {
            _locs.data()[_occupied].p = p;
            p->location() = &_locs.data()[_occupied];
            ++_occupied;
        }
        
        //! Read from the environment.
        template <typename Organism>
        int read(Organism& org, ea_type& ea) {
            return ea.rng()(std::numeric_limits<int>::max());
        }
        
        /*! Consume resources.
         
         Although conceptually simple, we pass in a few extra parameters to support
         eventual spatial resources.
         */
        double reaction(resource_ptr_type r, individual_type org, ea_type& ea) {
            return r->consume();
        }
        
        //! Add a resource to this environment.
        void add_resource(resource_ptr_type r) {
            _resources.push_back(r);
        }
        
        //! Fractional update.
        void partial_update(double delta_t, ea_type& ea) {
            for(resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->update(delta_t);
            }
        }
        
        location_matrix_type& locations() { return _locs; }
        
        //! Serialize this topology.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
        }
        
        int _occupied; //!< How many locations are currently occupied?
        location_matrix_type _locs; //!< List of all locations in this topology.
    };



} // ea

#endif
