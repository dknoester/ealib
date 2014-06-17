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

    /*! The position_type is contained by individuals to describe their position 
     and orientation in the environment.  It can be thought of as an index into 
     the environment.
     
     An individual's position is described by two vectors: r, which gives the
     coordinates of the individual in the environment, and h, which gives the
     orientation (heading) of the individual (i.e., it's "facing", in Avida
     parlance).
     
     A nice side effect of using vectors to describe position and orientation is
     that r+h gives the coordinates of the location that this position_type is
     facing.
     
     \note In some cases, r+h can result in negative values for x or y coordinates.
     That's ok; the environment uses a torus2 for storing locations, which allows
     negative indices.
     */
    struct position_type {
        //! Constructor.
        position_type(int xpos=0, int ypos=0, int xori=1, int yori=0) {
            r[0] = xpos; r[1] = ypos;
            h[0] = xori; h[1] = yori;
        }
        
        //! Copy constructor.
        position_type(const position_type& that) {
            r[0] = that.r[0]; r[1] = that.r[1];
            h[0] = that.h[0]; h[1] = that.h[1];
        }
        
        //! Assignment operator.
        position_type& operator=(const position_type& that) {
            if(this != &that) {
                r[0] = that.r[0]; r[1] = that.r[1];
                h[0] = that.h[0]; h[1] = that.h[1];
            }
            return *this;
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
         means that we can quickly calculate the x and y indices for the
         faced location (r+h).
         */
        void rotate(const double R[2][2]) {
            // [x;y] = Rh:
            double x = R[0][0] * static_cast<double>(h[0]) + R[0][1] * static_cast<double>(h[1]);
            double y = R[1][0] * static_cast<double>(h[0]) + R[1][1] * static_cast<double>(h[1]);
            // round "up" (really, away from zero), preserve sign:
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
    
    
    /*! Environment for individuals in a digital evolution algorithm.
     
     This environment provides a 2d torus in which individuals can interact.  It
     is primarily responsible for maintaining a spatial relationship among 
     individuals.  Specifically, the environment can be queried for neighbors.
     It also provides a mechanism for stigmergy, where each location is itself a
     metadata container.
     
     Conceptually, the environment provides a 2d torus of locations.  Each location
     can have at most one individual associated with it (its inhabitant).  The
     neighborhood of a location is the 8 locations that immediately surround it (a
     Moore neighborhood).
     
     A location_iterator is simply an iterator into the array backing the 2d torus,
     while a neighborhood_iterator lets one iterate over the locations surrounding
     a given location.
     
     Location iterators are important, as they provide a way for replacment operators
     to identify where in the environment an offspring should be placed.
     
     Neighborhood iterators, on the other hand, provide a way to examine the immediately
     surrounding area of a given individual (e.g., to find a location for replacement,
     or to send a message).
     
     By convention, coordinates in the torus are a 2-element array "r", with 
     r[0]==x and r[1]==y.  (Orientations are similar, and called "h").
     
     \todo It's probably best to do away with a fixed-size torus backing the
     environment, e.g., it could be replaced with a quadtree, but it works for now.
     */
    template <typename EA>
    class environment {
    public:
        typedef typename EA::individual_type individual_type;
        typedef typename EA::individual_ptr_type individual_ptr_type;

        /*! Location type.
         
         The environment is a 2d torus of locations; each location holds its
         coordinates and a metadata container (e.g., for stigmergy).
         */
        struct location_type {
            //! Constructor.
            location_type() {
                r[0] = 0; r[1] = 0;
            }
            
            //! Operator ==
            bool operator==(const location_type& that) {
                if((p==0) != (that.p==0)) { // pointer xor
                    return false;
                }
                
                bool t=true;
                if(p != 0) {
                    t = ((*p) == (*that.p));
                }
                
                return t && (r[0]==that.r[0])
                && (r[1]==that.r[1])
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
                return position_type(r[0], r[1]);
            }
            
            //! Serialize this location.
            template <class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                // we don't serialize the individual ptr - have to attach it after checkpoint load.
                ar & boost::serialization::make_nvp("r", r);
                ar & boost::serialization::make_nvp("metadata", _md);
            }
            
            typename EA::individual_ptr_type p; //!< Individual (if any) at this location.
            int r[2]; //!< (X,Y) coordinates of this location.
            metadata _md; //!< Meta-data container.
        };
        
        typedef torus2<location_type> location_storage_type;
        typedef typename location_storage_type::array_type::iterator location_iterator;
        typedef typename location_storage_type::array_type::const_iterator const_location_iterator;
        
        /*! This iterator is used to iterate over the locations in the neighborhood
         of a given location.
         
         \note Iteration begins at the currently-faced location, and proceeds ccw.
         */
        struct neighborhood_iterator : boost::iterator_facade<neighborhood_iterator, location_type, boost::single_pass_traversal_tag> {
            //! Constructor.
            neighborhood_iterator(const position_type& p, int c, location_storage_type& locs)
            : _pos(p), _count(c), _locs(locs) {
            }

            //! Increment operator.
            void increment() {
                _pos.rotate_ccw();
                ++_count;
            }
            
            //! Iterator equality comparison.
            bool equal(const neighborhood_iterator& that) const {
                return (_pos == that._pos) && (_count == that._count);
            }
            
            //! Dereference this iterator.
            location_type& dereference() const {
                return _locs(_pos.r[0]+_pos.h[0], _pos.r[1]+_pos.h[1]);
            }
            
            //! Get an iterator to the location this neighborhood iterator points to.
            location_iterator make_location_iterator() {
                location_type& l=dereference();
                return _locs.data().begin() + _locs.size2()*l.r[1] + l.r[0];
            }

            position_type _pos; //!< Position (origin location) of this iterator.
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
        
        //! Initializes the environment.
        void initialize(EA& ea) {
            assert((get<SPATIAL_X>(ea) * get<SPATIAL_Y>(ea)) <= get<POPULATION_SIZE>(ea));
            _locs.resize(get<SPATIAL_X>(ea), get<SPATIAL_Y>(ea), true);
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    _locs(i,j).r[0] = i;
                    _locs(i,j).r[1] = j;
                }
            }
        }
        
        //! Clears all individuals from the environment.
        void clear(EA& ea) {
            assert((get<SPATIAL_X>(ea) * get<SPATIAL_Y>(ea)) <= get<POPULATION_SIZE>(ea));
            _locs.resize(get<SPATIAL_X>(ea), get<SPATIAL_Y>(ea), true);
            for(std::size_t i=0; i<_locs.size1(); ++i) {
                for(std::size_t j=0; j<_locs.size2(); ++j) {
                    _locs(i,j).p.reset();
                }
            }
        }
        
        /*! Links the individuals in the existing population to their positions
         in the environment.
         
         This method should be used whenever the environment loses track of the
         individual pointers, e.g., upon deserialization or EA assignment.
         */
        void link(EA& ea) {
            for(typename EA::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
                operator[]((*i)->position()).p = *i;
            }
        }

        //! Insert individual p at the first available location.
        void insert(individual_ptr_type p, EA& ea) {
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
        }
        
        /*! Replaces an individual living at location i (if any) with
         individual p.  The individual's heading is set to 0.  If i is end(),
         sequentially search for the first available location.  If an available
         location cannot be found, throw an exception.
         */
        void replace(location_iterator i, individual_ptr_type p, EA& ea) {
            location_type& l=(*i);
            // kill the occupant of l, if any
            if(l.p) {
                l.p->alive() = false;
                ea.events().death(*l.p,ea);
            }
            l.p = p;
            p->position() = l.position();
        }

        //! Returns a location given a position.
        location_type& operator[](const position_type& pos) {
            return _locs(pos.r[0], pos.r[1]);
        }
        
        //! Returns a [begin,end) pair of iterators over an individual's neighborhood.
        std::pair<neighborhood_iterator,neighborhood_iterator> neighborhood(individual_type& p) {
            return std::make_pair(neighborhood_iterator(p.position(), 0, _locs),
                                  neighborhood_iterator(p.position(), 8, _locs));
        }
        
        //! Returns an iterator to the location currently faced by an individual.
        location_iterator neighbor(individual_ptr_type p) {
            return neighborhood_iterator(p->position(), 0, _locs).make_location_iterator();
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
        
    protected:
        location_storage_type _locs; //!< Torus of locations in this environment.

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
