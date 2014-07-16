/* spatial_graph.h
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
#ifndef _EA_LSYS_SPATIAL_GRAPH_H_
#define _EA_LSYS_SPATIAL_GRAPH_H_

#include <boost/graph/adjacency_list.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/tuple/tuple.hpp>
#include <vector>
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;


namespace ealib {
    namespace lsys {
        
        /*! This file provides primitives needed to "grow" a network a la
         Hermann Cuntz' paper, "One Rule to Grow Them All: A General Theory of
         Neuronal Branching and Its Practical Application," PLoS Comp. Bio.,
         6(8), 2010.
         */
        class spatial_graph2 {
        public:
            typedef bg::model::point<double, 2, bg::cs::cartesian> point_type;
            typedef std::list<point_type> point_list_type;
            typedef std::pair<point_type, point_list_type::iterator> value_type;
            typedef std::vector<value_type> query_result_type;
            typedef bgi::rtree<value_type, bgi::rstar<16> > rtree_type;
            
            //! Data structure to describe a carrier point.
            struct carrier_point_type {
                //! Default constructor.
                carrier_point_type() : root(0.0), point(0.0, 0.0) {
                }
                
                //! Constructor.
                carrier_point_type(const point_type& p) : root(0.0), point(p) {
                }
                
                //! Constructor.
                carrier_point_type(double x, double y) : root(0.0), point(x,y) {
                }
                double root; //!< Distance to root (path length).
                point_type point; //!< Location in space of this carrier point.
            };
            
            //! Type for the graph produced by neuron growth.
            typedef boost::adjacency_list
            < boost::setS
            , boost::vecS
            , boost::undirectedS
            , carrier_point_type
            > carrier_graph_type;
            
            //! Constructor.
            spatial_graph2() {
            }
            
            //! Add a point to this coordinate system.
            template <typename Point>
            void point(const Point& p) {
                point_list_type::iterator i = _cpoints.insert(_cpoints.end(), point_type(p(0), p(1)));
                _rtree.insert(std::make_pair(*i, i));
            }
            
            //! Outputs the K-nearest neighbors to point p.
            template <typename OutputIterator>
            void knn(const point_type& p, std::size_t k, OutputIterator oi) {
                _rtree.query(bgi::nearest(p, k), oi);
            }
            
            //! Outputs the points enclosed by box(p1,p2).
            template <typename OutputIterator>
            void enclosed(const point_type& p1, const point_type& p2, OutputIterator oi) {
                bg::model::box<point_type> query_box(p1, p2);
                _rtree.query(bgi::intersects(query_box), oi);
            }

            //! Returns the current graph representation of the points in this coordinate space.
            carrier_graph_type& graph() {
                return _G;
            }

            //! Returns the root point (by default, the first node).
            point_type& root() {
                return _cpoints.front();
            }
            
            //! Returns the cost of connecting point p to the carrier point n.
            double cost(double bf, const point_type& p, const carrier_point_type& n) {
                double d = bg::distance(p,n.point);
                return d + bf * (n.root+d);
            }
            
            /*! Returns a graph grown from the current list of carrier points
             with balancing factor bf.
             */
            void grow(double bf) {
                _G.clear();
                
                // watch out for empty lists:
                if(_cpoints.empty()) {
                    return;
                }
                
                // <point, vertex of min cost connection, cost>
                typedef boost::tuple<point_type, carrier_graph_type::vertex_descriptor, double> tuple_type;
                typedef std::list<tuple_type> connection_list;
                connection_list P;
                
                // first, add the root carrier point to the tree, which we assume
                // to be the first point in the list:
                point_list_type::iterator ci=_cpoints.begin();
                carrier_graph_type::vertex_descriptor root=boost::add_vertex(carrier_point_type(*ci), _G);
                
                // iterate through the rest of the carrier points and add
                // them to the connection list.  the root is the only node in the
                // graph at this point, so we initialize all costs in the connection
                // list to point to the root node.  As an optimization, keep track
                // of the minimum cost connection:
                connection_list::iterator mini=P.end();
                for( ++ci; ci!=_cpoints.end(); ++ci) {
                    connection_list::iterator last=P.insert(P.end(),
                                                            boost::make_tuple(*ci, root, cost(bf, *ci, _G[root])));
                    if((mini==P.end()) || (last->get<2>() < mini->get<2>())) {
                        mini = last;
                    }
                }
                
                // until all the points in P are exhausted, add the minimum cost
                // connection from P to its corresponding min cost vertex in _G:
                while(!P.empty()) {
                    // mini currently points at the minimum cost connection.
                    // add the corresponding vertex and edge to _G:
                    carrier_graph_type::vertex_descriptor u = mini->get<1>();
                    carrier_graph_type::vertex_descriptor v = boost::add_vertex(carrier_point_type(mini->get<0>()), _G);
                    boost::add_edge(u, v, _G);
                    _G[v].root = _G[u].root + bg::distance(_G[u].point, _G[v].point);
                    
                    // remove mini from the connection list and check to see
                    // if any points in P need to update their minimum to point
                    // to the vertex we just added:
                    P.erase(mini);
                    if(!P.empty()) {
                        mini = P.end();
                        for(connection_list::iterator i=P.begin(); i!=P.end(); ++i) {
                            // check to see if we need to update the cost of i
                            // based on the new node we just added.
                            //
                            // we only need to check all unconnected points in P
                            // against v (the node we just added).  we do not
                            // need to check against all vertices in _G.  By
                            // contradiction, this would imply that the minimums
                            // in P were wrong initially.
                            double c = cost(bf, i->get<0>(), _G[v]);
                            if(c < i->get<2>()) {
                                i->get<1>() = v;
                                i->get<2>() = c;
                            }
                            
                            // check to see if this is our current min cost
                            // connection:
                            if((mini==P.end()) || (i->get<2>() < mini->get<2>())) {
                                mini = i;
                            }
                        }
                    }
                }
            }
            
        protected:
            rtree_type _rtree; //!< Spatial index of carrier points.
            point_list_type _cpoints; //!< List of all carrier points.
            carrier_graph_type _G; //!< Graph version of carrier points.
        };
        
    } // lsys
} // ealib

#endif
