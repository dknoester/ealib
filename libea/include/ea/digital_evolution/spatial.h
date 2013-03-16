/* digital_evolution/spatial.h 
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
#include <stdexcept>
#include <ea/algorithm.h>
#include <ea/meta_data.h>

namespace ea {
    
    struct empty { };
    
    LIBEA_MD_DECL(SPATIAL_X, "ea.environment.x", int);
    LIBEA_MD_DECL(SPATIAL_Y, "ea.environment.y", int);
    
    
    namespace resources {
        //! Abstract resource type.
        struct abstract_resource {
            abstract_resource(const std::string& name) : _name(name) { }
            //! Update resource levels, if needed, based on elapsed time since last update (as a fraction of update length).
            virtual void update(double delta_t) = 0;
            //! Consume resources.
            virtual double consume() = 0;
            virtual void reset() = 0;
            
            std::string _name;
        };
        
        //! Unlimited resource type.
        struct unlimited : abstract_resource {
            unlimited(const std::string& name) : abstract_resource(name) { }
            void update(double) { }
            double consume() { return 1.0; }
            void reset() { }
        };
        
        //! Limited resource type.
        struct limited : abstract_resource {
            limited(const std::string& name, double initial, double inflow, double outflow, double consume)
            : abstract_resource(name), _initial(initial), _level(initial), _inflow(inflow), _outflow(outflow), _consume(consume) {
            }
            
            void update(double delta_t) {
                _level += delta_t * (_inflow - (_outflow * _level));
                _level = std::max(0.0, _level);
            }
            
            double consume() {
                double r = std::max(0.0, _level*_consume);
                _level = std::max (0.0, _level-r);
                return r;
            }
            
            void reset() {
                _level = _initial; 
            }

            double _initial; //!< Initial resource level
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
        
        
        /*! Type that is contained (and owned) by organisms to uniquely identify
         their location in the environment.
         
         \warning: This type must be serializable.
         */
        typedef std::pair<std::size_t, std::size_t> location_handle_type;

        /*! Location type.
         
         While locations logically "live" inside organisms, they are interpreted 
         by the specific topology being used.  So, the topology "owns" the various
         locations, but organisms have handles to the specific location at which
         they reside.
         
         Locations also have a pointer to their organism, thus we can go betwixt
         them with ease.
         */
        struct location_type {
            //! Constructor.
            location_type() : heading(0), x(0), y(0) {
            }
            
            //! Operator ==
            bool operator==(const location_type& that) {
                if((p==0) == (that.p!=0)) { // pointer xor...
                    return false;
                }
                
                bool r=true;
                if(p != 0) {
                    r = ((*p) == (*that.p));
                }
                
                return r && (heading==that.heading)
                && (x==that.x)
                && (y==that.y)
                && (_md==that._md);
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
            
            location_handle_type handle() { return std::make_pair(y,x); }
            
            individual_ptr_type p; //!< Individual (if any) at this location.
            int heading; //!< Heading of organism, in degrees.
            std::size_t x,y; //!< X-Y coordinates of this location.
            meta_data _md; //!< Meta-data container.

            //! Serialize this location.
            template <class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                ar & boost::serialization::make_nvp("heading", heading);
                ar & boost::serialization::make_nvp("x", x);
                ar & boost::serialization::make_nvp("y", y);
                ar & boost::serialization::make_nvp("meta_data", _md);
            }
        };
        
        typedef location_type* location_ptr_type;

        typedef boost::numeric::ublas::matrix<location_type> location_matrix_type; //! Container type for locations.
        
        location_ptr_type handle2ptr(const location_handle_type& handle) {
            return &_locs(handle.first, handle.second);
        }
        
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
        spatial() : _append_count(0) {
        }
        
        //! Operator==.
        bool operator==(const spatial& that) {
            if(_locs.size1() != that._locs.size1())
                return false;
            if(_locs.size2() != that._locs.size2())
                return false;
            bool r=true;
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    r = r && (_locs(i,j) == that._locs(i,j));
                    if(!r) {
                        return false;
                    }
                }
            }
            return true;
        }
        
        //! Initialize this topology.
        void initialize(ea_type& ea) {
            _locs.resize(get<SPATIAL_Y>(ea), get<SPATIAL_X>(ea), true);
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    _locs(i,j).x = j;
                    _locs(i,j).y = i;
                }
            }
        }
        
        //! Retrieve the neighborhood of the given individual.
        std::pair<iterator,iterator> neighborhood(individual_ptr_type p, ea_type& ea) {
            return std::make_pair(iterator(*handle2ptr(p->location()),0,_locs,ea),
                                  iterator(*handle2ptr(p->location()),8,_locs,ea));
        }
        
        //! Retrieve the currently faced neighboring location of the given individual.
        iterator neighbor(individual_ptr_type p, ea_type& ea) {
            return iterator(*handle2ptr(p->location()),handle2ptr(p->location())->heading,_locs,ea);
        }
        
        //! Given two orgs, rotate them to face one another.
        void face_org(individual_type& p1, individual_type& p2) {
            location_ptr_type l1 = p1.location();
            location_ptr_type l2 = p2.location();
        
            // Make sure everyone has a location...
            if ((l1 == NULL) || (l2 == NULL)) {
                return;
            }
            
           
            // think in terms of x,y. sort out later.
            if ((l1->x < l2->x) && (l1->y < l2->y)) {
                p1.location()->set_heading(1);
                p2.location()->set_heading(5);
                // l1 heading = 1
                // l2 heading = 5
            } else if ((l1->x > l2->x) && (l1->y > l2->y)) {
                // l1 heading = 5
                // l2 heading = 1
                p1.location()->set_heading(5);
                p2.location()->set_heading(1);
            } else if ((l1->x < l2->x) && (l1->y > l2->y)) {
                // l1 heading = 7
                // l2 heading = 3
                p1.location()->set_heading(7);
                p2.location()->set_heading(3);
            } else if ((l1->x > l2->x) && (l1->y > l2->y)) {
                // l1 heading = 3
                // l2 heading = 7
                p1.location()->set_heading(3);
                p2.location()->set_heading(7);
            } else if ((l1->x < l2->x) && (l1->y == l2->y)) {
                // l1 heading = 0
                // l2 heading = 4
                p1.location()->set_heading(0);
                p2.location()->set_heading(4);
            } else if ((l1->x > l2->x) && (l1->y == l2->y)) {
                // l1 heading = 4
                // l2 heading = 0
                p1.location()->set_heading(4);
                p2.location()->set_heading(0);
            } else if ((l1->x == l2->x) && (l1->x < l2->y)) {
                // l1 heading = 2
                // l2 heading = 6
                p1.location()->set_heading(2);
                p2.location()->set_heading(6);
            } else if ((l1->x == l2->x) && (l1->x < l2->y)) {
                // l1 heading = 6
                // l2 heading = 2
                p1.location()->set_heading(6);
                p2.location()->set_heading(2);
            }
            
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
            p->location() = l.handle();
        }

        //! Append individual x to the environment.
        void append(individual_ptr_type p) {
            if(_append_count >= (_locs.size1()*_locs.size2())) {
                throw std::out_of_range("spatial::append(individual_ptr_type x)");
            }
            _locs.data()[_append_count].p = p;
            p->location() = _locs.data()[_append_count].handle();
            ++_append_count;
        }
        
        //! Append the range of individuals [f,l) to the environment.
        template <typename ForwardIterator>
        void append(ForwardIterator f, ForwardIterator l) {
            for( ; f!=l; ++f) {
                append(*f);
            }
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
        
        /*! Reset resources -- may occur on successful group event 
         */
        void reset_resources() {
            for(resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->reset();
            }
        }
        
        location_matrix_type& locations() { return _locs; }
        
        location_type& location(int x, int y) {
            // x == j, y == i
            return _locs(y, x);
        }
        
        /*! Serialize this topology.
         
         \warning: this leaves the pointer to the individual (in each location)
         unconnected.  this has to be fixed up after deserialization.
         */
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("locations", _locs);
            ar & boost::serialization::make_nvp("append_count", _append_count);
        }
        
        /*! This is called after deserialization (load); the idea here is that we
         need to iterate through the population, and link the locations to their
         respective organisms.
         */
        void attach(ea_type& ea) {
            for(typename ea_type::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
                handle2ptr((*i)->location())->p = *i;
            }
        }

        std::size_t _append_count; //!< Number of locations that have been appended to.
        location_matrix_type _locs; //!< Matrix of all locations in this topology.
    };
} // ea

#endif
