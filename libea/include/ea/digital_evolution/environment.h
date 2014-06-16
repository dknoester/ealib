/* digital_evolution/discrete_spatial_environment.h 
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

#ifndef _EA_DIGITAL_EVOLUTION_ENVIRONMENT_H_
#define _EA_DIGITAL_EVOLUTION_ENVIRONMENT_H_

#include <boost/iterator/iterator_facade.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <utility>
#include <vector>

#include <ea/algorithm.h>
#include <ea/metadata.h>
#include <ea/data_structures/torus2.h>

namespace ealib {

    /*! Type that is contained by individuals to describe their position and
     orientation in the environment.
     */
    struct position_type {
        //! Constructor.
        position_type(int xpos=0, int ypos=0, int xatt=1, int yatt=0) {
            r[0] = xpos; r[1] = ypos;
            h[0] = xatt; h[1] = yatt;
        }
        
        //! Copy constructor.
        position_type(const position_type& that) {
            r[0] = that.r[0]; r[1] = that.r[1];
            h[0] = that.h[0]; h[1] = that.h[1];
        }
        
        //! Operator==.
        bool operator==(const position_type& that) const {
            return (r[0] == that.r[0]) && (r[1] == that.r[1]) && (h[0] == that.h[0]) && (h[1] == that.h[1]);
        }
        
        /*! Rotate this position by matrix R.

         Specifically, multiply this position's heading vector h by R.  This
         enables a quick lookup of the neighboring location, and avoids large
         switch statements on heading.  The math is pretty straightforward:
         
         h' = R*h,
         
         where R is a rotation matrix, h is the current heading vector, and h' is
         the new heading vector.  For reference, rotation matrices are:
         R = [ cos\theta  -sin\theta ]
             [ sin\theta   cos\theta ]
         
         where \theta is the angle by which we're rotating (in radians).
         
         We go to some length to make sure that h is integer-valued, which also
         means that we can quickly calculate the x and y offsets for the neighboring
         location (x+h[0], y+h[1]).
         */
        void rotate(const double R[2][2]) {
            double x = R[0][0] * static_cast<double>(h[0]) + R[0][1] * static_cast<double>(h[1]);
            double y = R[1][0] * static_cast<double>(h[0]) + R[1][1] * static_cast<double>(h[1]);
            h[0] = std::copysign(static_cast<int>(fabs(x) + 0.5), x);
            h[1] = std::copysign(static_cast<int>(fabs(y) + 0.5), y);
        }
        
        //! Rotate by theta radians.
        void rotate(double theta) {
            double R[2][2];
            R[0][0] = R[1][1] = cos(theta);
            R[0][1] = -sin(theta);
            R[1][0] = -R[0][1];
            rotate(R);
        }

        //! Convenience method to rotate ccw by pi/4 radians.
        void rotate_ccw() {
            static const double R[2][2] = {{0.707, -0.707}, {0.707, 0.707}};
            rotate(R);
        }
        
        //! Convenience method to rotate cw by pi/4 radians.
        void rotate_cw() {
            static const double R[2][2] = {{0.707, 0.707}, {-0.707, 0.707}};
            rotate(R);
        }
        
        //! Serialize this position.
		template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("r", r);
            ar & boost::serialization::make_nvp("h", h);
		}

        int r[2]; //!< Individual's position vector (x,y).
        int h[2]; //!< Individual's orientation vector (x,y).
    };
    
    
    /*! Location type.
     
     An individual's position in the environment can best be thought of as
     an index into a location data structure which contains locale-specific
     information; this is the locale-specific data.
     */
    template <typename EA>
    struct environment_location {
        //! Constructor.
        environment_location() : x(0), y(0) {
        }
        
        //! Operator ==
        bool operator==(const environment_location& that) {
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
        metadata& md() { return _md; }
        
        //! Is this location occupied?
        bool occupied() { return ((p != 0) && (p->alive())); }
        
        //! Return the inhabitant.
        typename EA::individual_ptr_type inhabitant() { return p; }

        //! Returns a position_type for this location.
        position_type position() {
            return position_type(x,y);
        }
        
        //! Serialize this location.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            // we don't serialize the individual ptr - have to attach it after checkpoint load.
            ar & boost::serialization::make_nvp("x", x);
            ar & boost::serialization::make_nvp("y", y);
            ar & boost::serialization::make_nvp("metadata", _md);
        }
        
        typename EA::individual_ptr_type p; //!< Individual (if any) at this location.
        std::size_t x,y; //!< X-Y coordinates of this location.
        metadata _md; //!< Meta-data container.
    };
    

    
    /*! Discrete spatial environment.
     
     */
    template <typename EA>
    class environment {
    public:
        typedef environment_location<EA> location_type;
        typedef torus2<location_type> location_storage_type;
        typedef typename location_storage_type::array_type::iterator location_iterator;
        typedef typename location_storage_type::array_type::const_iterator const_location_iterator;
        typedef typename EA::individual_type individual_type;
        typedef typename EA::individual_ptr_type individual_ptr_type;
        
        /*! This iterator is used to iterate over the neighborhood of a position
         in the environment (the 8 surrounding locations; a Moore neighborhood).
         */
        struct neighborhood_iterator : boost::iterator_facade<neighborhood_iterator, location_type, boost::single_pass_traversal_tag> {
            //! Constructor.
            neighborhood_iterator(const position_type& pos, int c, location_storage_type& locs)
            : _p(pos), _count(c), _locs(locs) {
            }
            
            //! Increment operator.
            void increment() {
                _p.rotate_ccw();
                ++_count;
            }
            
            //! Iterator equality comparison.
            bool equal(const neighborhood_iterator& that) const {
                return (_p == that._p) && (_count == that._count);
            }
            
            //! Dereference this iterator.
            location_type& dereference() const {
                return _locs(_p.r[0]+_p.h[0], _p.r[1]+_p.h[1]);
            }
            
            //! Get an iterator to the location this neighborhood iterator points to.
            location_iterator make_location_iterator() {
                location_type& l=dereference();
                return _locs.data().begin() + _locs.size2()*l.y + l.x;
            }

            position_type _p; //!< Position of the origin of this iterator.
            int _count; //!< Increment count for this iterator, used to check end.
            location_storage_type& _locs; //!< Location storage.
        };
        
        //! Constructor.
        environment() {
        }
        
        //! Operator==.
        bool operator==(const environment& that) {
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
        void initialize(EA& ea) {
            assert((get<SPATIAL_X>(ea) * get<SPATIAL_Y>(ea)) <= get<POPULATION_SIZE>(ea));
            _locs.resize(get<SPATIAL_X>(ea), get<SPATIAL_Y>(ea), true);
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    _locs(i,j).x = i;
                    _locs(i,j).y = j;
                }
            }
        }
        
        //! Returns an iterator to the beginning of this environment.
        location_iterator begin() {
            return _locs.data().begin();
        }
        
        //! Returns an iterator to the end of this environment.
        location_iterator end() {
            return _locs.data().end();
        }
        
        /*! Replaces an individual living at location i (if any) with
         individual p.  The individual's heading is set to 0.  If i is end(),
         sequentially search for the first available location.  If an available
         location cannot be found, throw an exception.
         */
        void replace(location_iterator i, individual_ptr_type p, EA& ea) {
            if(i == end()) {
                // search for an available location in the environment;
                // by default, this insertion is sequential.
                for(std::size_t i=0; i<_locs.size1(); ++i) {
                    for(std::size_t j=0; j<_locs.size2(); ++j) {
                        location_type& l=_locs(i,j);
                        if(!l.occupied()) {
                            l.p = p;
                            p->position() = l.position();
                            return;
                        }
                    }
                }
                // if we get here, the environment is full; throw.
                throw fatal_error_exception("environment: could not find available location");
            } else {
                location_type& l=(*i);
                // kill the occupant of l, if any
                if(l.p) {
                    l.p->alive() = false;
                    ea.events().death(*l.p,ea);
                }
                l.p = p;
                p->position() = l.position();
            }
        }

        //! Swap individuals (if any) betweeen locations i and j.
        void swap_locations(std::size_t i, std::size_t j) {
            assert(i < (_locs.size1()*_locs.size2()));
            assert(j < (_locs.size1()*_locs.size2()));
            location_type& li=_locs.data()[i];
            location_type& lj=_locs.data()[j];

            // swap individual pointers:
            std::swap(li.p, lj.p);

            // and fixup positions:
            if(li.occupied()) {
                li.p->position() = li.position();
            }
            if(lj.occupied()) {
                lj.p->position() = lj.position();
            }
        }
        
        //! Returns a location given a position.
        location_type& location(const position_type& pos) {
            return _locs(pos.r[0], pos.r[1]);
        }

        //! Returns a location given x and y coordinates.
        location_type& location(int x, int y) {
            return _locs(x, y);
        }
        
        //! Rotates two individuals to face one another.
        void face_org(individual_type& ind1, individual_type& ind2) {
            position_type& p1 = ind1.position();
            position_type& p2 = ind2.position();

            int x = p1.r[0] - p2.r[0];
            int y = p1.r[1] - p2.r[1];
            
            assert((x>=-1) && (x<=1));
            assert((y>=-1) && (y<=1));
            
            p1.h[0] = -x; p2.h[0] = x;
            p1.h[1] = -y; p2.h[1] = y;
        }

        //! Returns a [begin,end) pair of iterators over an individual's neighborhood.
        std::pair<neighborhood_iterator,neighborhood_iterator> neighborhood(individual_type& p) {
            return std::make_pair(neighborhood_iterator(p.position(), 0, _locs),
                                  neighborhood_iterator(p.position(), 8, _locs));
        }
        
        //! Returns the location currently faced by an organism.
        location_iterator neighbor(individual_ptr_type p) {
            return neighborhood_iterator(p->position(), 0, _locs).make_location_iterator();
        }
        
        /*! Called after load (deserialization) to attach the environment to
         the population.  This sets the individual_ptr_type held by each location.
         */
        void after_load(EA& ea) {
            for(typename EA::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
                location((*i)->position()).p = *i;
            }
        }

    protected:
        location_storage_type _locs; //!< Matrix of all locations in this topology.

    private:
		friend class boost::serialization::access;
        template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            std::size_t size1=_locs.size1();
            std::size_t size2=_locs.size2();
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
