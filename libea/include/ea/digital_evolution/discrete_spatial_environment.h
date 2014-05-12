/* digital_evolution/discrete_spatial_environment.h 
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
#include <limits>
#include <utility>
#include <vector>
#include <stdexcept>

#include <ea/algorithm.h>
#include <ea/meta_data.h>
#include <ea/digital_evolution/position.h>
#include <ea/digital_evolution/resources.h>

namespace ealib {
    
    /*! Discrete spatial environment.
     
     This spatial environment is divided into discrete cells.
     
     ON ORIENTATION:
     This environment is oriented as the standard X-Y Cartesian coordinate
     system.  I.e., (0,0) is in the lower left, positive is up and right, etc.
     
     ON POSITION:
     Positions in this space are a triple (x, y, heading), where heading is a
     number in the range [0,8), such that for a given position Origin (Or.),
     headings point in the following directions:
     
     3  |  2  |  1
     4  |  Or.|  0
     5  |  6  |  7
     
     
     */
    template <typename EA>
    class discrete_spatial_environment {
    public:
        //! EA type using this topology.
        typedef EA ea_type;
        typedef typename ea_type::individual_type individual_type; //!< Individual type.
        typedef typename ea_type::individual_ptr_type individual_ptr_type; //!< Pointer to individual type.
        typedef boost::shared_ptr<resources::abstract_resource> resource_ptr_type; //!< Abstract resource type.
        typedef std::vector<resource_ptr_type> resource_list_type; //!< List of resources.

        /*! Location type.
         
         An individual's position in the environment can best be thought of as
         an index into a location data structure which contains locale-specific
         information; this is that data structure.
         */
        struct location_type {
            //! Constructor.
            location_type() : x(0), y(0) {
            }
            
            //! Operator ==
            bool operator==(const location_type& that) {
                if((p==0) != (that.p==0)) { // pointer xor...
                    return false;
                }
                
                bool r=true;
                if(p != 0) {
                    r = ((*p) == (*that.p));
                }
                
                return r && (x==that.x)
                && (y==that.y)
                && (_md==that._md);
            }
            
            //! Location meta-data.
            meta_data& md() { return _md; }
            
            //! Is this location occupied?
            bool occupied() { return ((p != 0) && (p->alive())); }
            
            //! Return the inhabitant.
            individual_ptr_type inhabitant() { return p; }
            
            //! Sets the heading of this location's inhabitant.
            void set_heading(int h) {
                if(occupied()) {
                    p->position()[HEADING] = h % 8;
                }
            }
            
            //! Alters the heading of this location's inhabitant.
            void alter_heading(int h) {
                if(occupied()) {
                    p->position()[HEADING] = algorithm::roll(p->position()[HEADING]+h, 0, 7);
                }
            }

            //! Returns this location's position vector.
            position_type position() {
                return make_position(x,y);
            }
            
            //! Serialize this location.
            template <class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                // we don't serialize the individual ptr - have to attach it after checkpoint load.
                ar & boost::serialization::make_nvp("x", x);
                ar & boost::serialization::make_nvp("y", y);
                ar & boost::serialization::make_nvp("meta_data", _md);
            }

            individual_ptr_type p; //!< Individual (if any) at this location.
            std::size_t x,y; //!< X-Y coordinates of this location.
            meta_data _md; //!< Meta-data container.
        };

        //! Location pointer type.
        typedef location_type* location_ptr_type;
        //! Location matrix type.
        typedef boost::numeric::ublas::matrix<location_type> location_matrix_type;
        
        //! Iterator for the neighborhood of a position.
        struct iterator : boost::iterator_facade<iterator, location_type, boost::single_pass_traversal_tag> {
            //! Constructor.
            iterator(position_type& pos, int h, location_matrix_type& locs)
            : _origin(locs(pos[XPOS], pos[YPOS])), _heading(h), _locs(locs) {
            }
            
            //! Increment operator.
            void increment() { ++_heading; }
            
            //! Iterator equality comparison.
            bool equal(iterator const& that) const {
                return (_origin.y==that._origin.y) && (_origin.x==that._origin.x) && (_heading==that._heading);
            }
            
            /*! Dereference this iterator.
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
                
                return _locs(x,y);
            }
            
            location_type& _origin; //!< Origin of this iterator.
            int _heading; //!< Current heading for the iterator.
            location_matrix_type& _locs; //!< list of all possible locations.
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
        
        //! Initialize this environment.
        void initialize(ea_type& ea) {
            assert((get<SPATIAL_X>(ea) * get<SPATIAL_Y>(ea)) <= get<POPULATION_SIZE>(ea));
            _locs.resize(get<SPATIAL_X>(ea), get<SPATIAL_Y>(ea), true);
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    _locs(i,j).x = i;
                    _locs(i,j).y = j;
                }
            }
        }
        
        //! Inserts individual p into the environment at index i.
        void insert_at(std::size_t i, individual_ptr_type p) {
            assert(i < (_locs.size1()*_locs.size2()));
            _locs.data()[i].p = p;
            p->position() = _locs.data()[i].position();
        }
        
        //! Inserts the range of individuals [f,l) into the environment starting at index i.
        template <typename ForwardIterator>
        void insert_at(std::size_t i, ForwardIterator f, ForwardIterator l) {
            for( ; f!=l; ++f, ++i) {
                insert_at(i,*f);
            }
        }
        
        //! Move an individual from position i to position j.
        void move_ind(std::size_t i, std::size_t j) {
            assert(i < (_locs.size1()*_locs.size2()));
            assert(j < (_locs.size1()*_locs.size2()));
            assert(_locs.data()[i].occupied());
            
            _locs.data()[j].p = _locs.data()[i].p;
            _locs.data()[i].p.reset();


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
            p->position() = l.position();
        }
        
        //! Returns a value "read" from this environment.
        template <typename Organism>
        int read(Organism& org, ea_type& ea) {
            return ea.rng()(std::numeric_limits<int>::max());
        }
        
        //! Returns the list of resources.
        resource_list_type& resources() { return _resources; }
        
        //! Adds a new resource to this environment.
        void add_resource(resource_ptr_type r) {
            _resources.push_back(r);
        }
        
        //! Clears all resource levels.
        void clear_resources() {
            for(typename resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->clear();
            }
        }
        
        //! Individual ind consumes resource r.
        double consume_resource(resource_ptr_type r, individual_type& ind) {
            return r->consume(ind.position());
        }
        
        //! Updates resource levels based on delta t.
        void update_resources(double delta_t) {
            for(typename resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->update(delta_t);
            }
        }
        
        //! Reset resources -- may occur on successful group event
        void reset_resources() {
            for(typename resource_list_type::iterator i=_resources.begin(); i!=_resources.end(); ++i) {
                (*i)->reset();
            }
        }

        //! Returns a location pointer given a position.
        location_ptr_type location(const position_type& pos) {
            return &_locs(pos[XPOS], pos[YPOS]);
        }

        //! Returns a location pointer given x and y coordinates.
        location_ptr_type location(int x, int y) {
            return &_locs(x, y);
        }
        
        //! Rotates two individuals to face one another.
        void face_org(individual_type& p1, individual_type& p2) {
            location_ptr_type l1 = location(p1.position());
            location_ptr_type l2 = location(p2.position());
            
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

        //! Returns a [begin,end) pair of iterators over an individual's neighborhood.
        std::pair<iterator,iterator> neighborhood(individual_type& p) {
            return std::make_pair(iterator(p.position(), 0, _locs),
                                  iterator(p.position(), 8, _locs));
        }
        
        //! Returns an iterator to the location in the specified direction from p.
        iterator neighbor(individual_type p, int dir) {
            assert(dir >= 0 && dir < 8);
            return iterator(p.position(), dir, _locs);
        }
        
        //! Returns an iterator to the currently faced neighboring location.
        iterator neighbor(individual_ptr_type p) {
            return iterator(p->position(), p->position()[HEADING], _locs);
        }
        
        /*! Called after load (deserialization) to attach the environment to
         the population.  This sets the individual_ptr_type held by each location.
         */
        void after_load(ea_type& ea) {
            for(typename ea_type::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
                location((*i)->position())->p = *i;
            }
        }

    protected:
        std::size_t _append_count; //!< Number of locations that have been appended to.
        location_matrix_type _locs; //!< Matrix of all locations in this topology.
        resource_list_type _resources;

    private:
		friend class boost::serialization::access;
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

} // ealib

#endif
