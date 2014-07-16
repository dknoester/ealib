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

            //! Data structure to describe a carrier point.
            struct carrier_point_type {
                carrier_point_type() : root(0.0), point(0.0, 0.0) { }
                carrier_point_type(const point_type& p) : root(0.0), point(p) { }
                carrier_point_type(double x, double y) : root(0.0), point(x,y) { }

                double root; //!< Distance to root (path length).
                point_type point; //!< Location in space of this carrier point.
            };

            //! Type for the underlying graph connecting carrier points.
            typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, carrier_point_type> graph_type;
            typedef std::pair<point_type, graph_type::vertex_descriptor> value_type; //!< Type that is stored in the RTree.
            typedef std::vector<value_type> query_result_type; //!< Type for storing spatial query results.
            typedef bgi::rtree<value_type, bgi::rstar<16> > rtree_type;
            
            //! Constructor.
            spatial_graph2() {
            }
            
            //! Add a point to this coordinate system.
            template <typename Point>
            void point(const Point& p) {
                graph_type::vertex_descriptor v = boost::add_vertex(carrier_point_type(p(0), p(1)), _G);
                _rtree.insert(std::make_pair(_G[v].point, v));
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
            graph_type& graph() {
                return _G;
            }

            //! Returns the carrier point corresponding to vertex v.
            carrier_point_type& carrier_point(graph_type::vertex_descriptor v) {
                return _G[v];
            }
            
            //! Returns the root point (by default, the first node).
            carrier_point_type& root() {
                return _G[boost::vertex(0, _G)];
            }
            
            /*! Returns the cost of connecting carrier point u to the tree at
             carrier point v.
             */
            double cost(double bf, const graph_type::vertex_descriptor u, const graph_type::vertex_descriptor v) {
                return bg::distance(_G[u].point, _G[v].point) + bf * _G[v].root;
            }
            
            //! Grow the tree.
            void grow(double bf) {
                if(boost::num_vertices(_G) == 0) {
                    return;
                }
                
                // <source vertex, target vertex of min cost connection, cost>
                typedef boost::tuple<graph_type::vertex_descriptor, graph_type::vertex_descriptor, double> tuple_type;
                typedef std::list<tuple_type> connection_list;
                connection_list P;
                
                // the root is the 0'th vertex:
                graph_type::vertex_descriptor root=boost::vertex(0, _G);
                
                // iterate through the non-root carrier points and add
                // them to the connection list.  the root is the only node in the
                // tree at this point, so we initialize all costs in the connection
                // list to point to the root node, keeping track of the minimum
                // cost connection:
                connection_list::iterator mini=P.end();
                for(std::size_t i=1; i<boost::num_vertices(_G); ++i) {
                    graph_type::vertex_descriptor u=boost::vertex(i,_G);
                    connection_list::iterator last=P.insert(P.end(),
                                                            boost::make_tuple(u, root, cost(bf, u, root)));
                    if((mini==P.end()) || (last->get<2>() < mini->get<2>())) {
                        mini = last;
                    }
                }
                
                // add all the connections in P in min-cost order:
                while(!P.empty()) {
                    // mini currently points at the minimum cost connection.
                    graph_type::vertex_descriptor u = mini->get<0>(); // source
                    graph_type::vertex_descriptor v = mini->get<1>(); // target
                    boost::add_edge(u, v, _G);
                    _G[u].root = _G[v].root + bg::distance(_G[u].point, _G[v].point);
                    
                    // remove mini from the connection list and check to see
                    // if any points in P need to update their minimum to point
                    // to vertex u (the one we just added):
                    P.erase(mini);
                    if(!P.empty()) {
                        mini = P.end();
                        for(connection_list::iterator i=P.begin(); i!=P.end(); ++i) {
                            // check to see if we need to update the cost of
                            // connection i based on the node we just added.
                            //
                            // we only need to check all unconnected points in P
                            // against u, not everything in _G.  By contradiction,
                            // this would imply that the minimum in P was wrong
                            // initially.
                            double c = cost(bf, i->get<0>(), u);
                            if(c < i->get<2>()) {
                                i->get<1>() = u;
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
            graph_type _G; //!< Graph version of carrier points.
        };
        
    } // lsys
} // ealib

#endif
