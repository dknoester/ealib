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

#ifndef _EA_DIGITAL_EVOLUTION_DISCRETE_SPATIAL_ENVIRONMENT_H_
#define _EA_DIGITAL_EVOLUTION_DISCRETE_SPATIAL_ENVIRONMENT_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <utility>
#include <vector>
#include <stdexcept>

#include <ea/algorithm.h>
#include <ea/meta_data.h>
#include <ea/digital_evolution/position.h>
#include <ea/digital_evolution/resources.h>

LIBEA_MD_DECL(SPATIAL_X, "ea.environment.x", int);
LIBEA_MD_DECL(SPATIAL_Y, "ea.environment.y", int);

namespace ealib {
    
    /*! Discrete spatial environment.
     
     This environment is divided into discrete cells.
     */
    template <typename EA>
    class discrete_spatial_environment {
    public:
        //! EA type using this topology.
        typedef EA ea_type;
        
        typedef typename ea_type::individual_ptr_type individual_ptr_type; //!< Pointer to individual type.
        
        typedef typename ea_type::individual_type individual_type; //!< Pointer to individual type.
        
        typedef boost::shared_ptr<resources::abstract_resource> resource_ptr_type;
        typedef std::vector<resource_ptr_type> resource_list_type;
        resource_list_type _resources;
        
        resource_list_type& resources() { return _resources; }
        
        void clear_resources() {
            for(typename resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->clear();
            }
        }
        
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
            
            position_type handle() { return std::make_pair(y,x); }
            
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
        
        location_ptr_type handle2ptr(const position_type& pos) {
            return &_locs(pos[0], pos[1]);
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
        discrete_spatial_environment() : _append_count(0) {
        }
        
        //! Operator==.
        bool operator==(const discrete_spatial_environment& that) {
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
        
        //! Retrieve the neighbor at the specified direction
        iterator direction_neighbor(individual_type p, int dir, ea_type& ea) {
            assert(dir < 8 && dir >= 0);
            return (iterator(*handle2ptr(p.location()), dir, _locs, ea));
        }
        
        //! Retrieve the currently faced neighboring location of the given individual.
        iterator neighbor(individual_ptr_type p, ea_type& ea) {
            return iterator(*handle2ptr(p->location()),handle2ptr(p->location())->heading,_locs,ea);
        }
        
        //! Given two orgs, rotate them to face one another.
        void face_org(individual_type& p1, individual_type& p2) {
            location_ptr_type l1 = handle2ptr(p1.location());
            location_ptr_type l2 = handle2ptr(p2.location());
            
            // Make sure everyone has a location...
            if ((l1 == NULL) || (l2 == NULL)) {
                return;
            }
            
            // think in terms of x,y. sort out later.
            if ((l1->x < l2->x) && (l1->y < l2->y)) {
                l1->set_heading(1);
                l2->set_heading(5);
                // l1 heading = 1
                // l2 heading = 5
            } else if ((l1->x > l2->x) && (l1->y > l2->y)) {
                // l1 heading = 5
                // l2 heading = 1
                l1->set_heading(5);
                l2->set_heading(1);
            } else if ((l1->x < l2->x) && (l1->y > l2->y)) {
                // l1 heading = 7
                // l2 heading = 3
                l1->set_heading(7);
                l2->set_heading(3);
            } else if ((l1->x > l2->x) && (l1->y > l2->y)) {
                // l1 heading = 3
                // l2 heading = 7
                l1->set_heading(3);
                l2->set_heading(7);
            } else if ((l1->x < l2->x) && (l1->y == l2->y)) {
                // l1 heading = 0
                // l2 heading = 4
                l1->set_heading(0);
                l2->set_heading(4);
            } else if ((l1->x > l2->x) && (l1->y == l2->y)) {
                // l1 heading = 4
                // l2 heading = 0
                l1->set_heading(4);
                l2->set_heading(0);
            } else if ((l1->x == l2->x) && (l1->x < l2->y)) {
                // l1 heading = 2
                // l2 heading = 6
                l1->set_heading(2);
                l2->set_heading(6);
            } else if ((l1->x == l2->x) && (l1->x < l2->y)) {
                // l1 heading = 6
                // l2 heading = 2
                l1->set_heading(6);
                l2->set_heading(2);
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
        double reaction(resource_ptr_type r, individual_type& org, ea_type& ea) {
            return r->consume(org,ea);
        }
        
        //! Add a resource to this environment.
        void add_resource(resource_ptr_type r) {
            _resources.push_back(r);
        }
        
        //! Fractional update.
        void partial_update(double delta_t, ea_type& ea) {
            for(typename resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->update(delta_t);
            }
        }
        
        /*! Reset resources -- may occur on successful group event 
         */
        void reset_resources() {
            for(typename resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->reset();
            }
        }
        
        location_matrix_type& locations() { return _locs; }
        
        location_type& location(int x, int y) {
            // x == j, y == i
            return _locs(y, x);
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

    protected:
        std::size_t _append_count; //!< Number of locations that have been appended to.
        location_matrix_type _locs; //!< Matrix of all locations in this topology.

    private:
		friend class boost::serialization::access;
        
        /*! Serialize this environment.
         
         \warning: this leaves the pointer to the individual (in each location)
         unconnected.  this has to be fixed up after deserialization.
         
         \warning: This is split up to avoid a compiler bug triggered somewhere
         in the boost serialization code.
         */
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            std::size_t size1=_locs.size1();
            std::size_t size2=_locs.size2();
            ar & boost::serialization::make_nvp("append_count", _append_count);
            ar & boost::serialization::make_nvp("size1", size1);
            ar & boost::serialization::make_nvp("size2", size2);
            
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    ar & boost::serialization::make_nvp("location", _locs(i,j));
                }
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            std::size_t size1=0, size2=0;
            ar & boost::serialization::make_nvp("append_count", _append_count);
            ar & boost::serialization::make_nvp("size1", size1);
            ar & boost::serialization::make_nvp("size2", size2);

            _locs.resize(size1,size2);
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    ar & boost::serialization::make_nvp("location", _locs(i,j));
                }
            }
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
} // ea

#endif
