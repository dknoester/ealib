/* cartesian_cs.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
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
#ifndef _EA_LSYS_CARTESIAN_H_
#define _EA_LSYS_CARTESIAN_H_

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace ealib {
    namespace lsys {
        
        /*! 2D Cartesian coordinate system.
         
         Note: At the moment, this coordinate system only supports adding points.
         */
        class cartesian2 {
        public:
            typedef bg::model::point<double, 2, bg::cs::cartesian> point_type;
            typedef std::pair<point_type, std::size_t> value_type;
            typedef std::vector<value_type> object_vector_type;
            typedef bgi::rtree<value_type, bgi::rstar<16> > rtree_type;
            
            //! Constructor.
            cartesian2() {
            }
            
            template <typename Point>
            void point(const Point& p) {
                value_type v = std::make_pair(point_type(p(0), p(1)), _objects.size());
                _objects.push_back(v);
                _rtree.insert(v);
            }
            
            template <typename OutputIterator>
            void knn(const point_type& p, std::size_t n, OutputIterator oi) {
                _rtree.query(bgi::nearest(p, 5), oi);
            }
            
            template <typename OutputIterator>
            void enclosed(const point_type& p1, const point_type& p2, OutputIterator oi) {
                bg::model::box<point_type> query_box(p1, p2);
                _rtree.query(bgi::intersects(query_box), oi);
            }
            
        protected:
            object_vector_type _objects; //!< Raw storage for locatable objects.
            rtree_type _rtree; //!< R-Tree, stores locations.
        };
        
    } // lsys
} // ealib

#endif
