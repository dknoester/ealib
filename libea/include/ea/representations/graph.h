/* graph.h
 *
 * This file is part of EALib.
 *
 * Copyright 2012 David B. Knoester.
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
#ifndef _EA_GRAPH_H_
#define _EA_GRAPH_H_

#include <boost/graph/adjacency_list.hpp>
#include <ea/meta_data.h>

LIBEA_MD_DECL(GRAPH_RESERVED_VERTICES, "graph.reserved_vertices", int);
LIBEA_MD_DECL(GRAPH_INITIAL_EDGES, "graph.edge.initial", int);
LIBEA_MD_DECL(GRAPH_ADD_EDGE_P, "graph.edge.add.p", double);
LIBEA_MD_DECL(GRAPH_REMOVE_EDGE_P, "graph.edge.remove.p", double);
LIBEA_MD_DECL(GRAPH_MUTATE_EDGE_P, "graph.edge.mutate.p", double);
LIBEA_MD_DECL(GRAPH_MUTATE_VERTEX_P, "graph.vertex.mutate.p", double);


namespace ea {
    namespace mutation {
        namespace detail {
            
            //! Add an edge (may create a new vertex):
            template <typename Representation, typename EA>
            void add_edge(Representation& G, EA& ea) {
                std::size_t un,vn;
                // the "+1"th vertex is the "add a vertex" button:
                boost::tie(un,vn) = ea.rng().choose_two(0, static_cast<int>(boost::num_vertices(G)+1));
            
                typename Representation::vertex_descriptor u=boost::vertex(un,G);
                typename Representation::vertex_descriptor v;
                if(vn == boost::num_vertices(G)) {
                    v = boost::add_vertex(G);
                } else {
                    v = boost::vertex(vn,G);
                }
            
                boost::add_edge(u, v, G);
            }
            
            
            //! Remove an edge (may remove a vertex):
            template <typename Representation, typename EA>
            void remove_edge(Representation& G, EA& ea) {
                // find a random edge:
                typename Representation::edge_iterator ei,ei_end;
                boost::tie(ei,ei_end) = boost::edges(G);
                ei = ea.rng().choice(ei,ei_end);

                // get the vertices on either side of this edge:
                typename Representation::vertex_descriptor u=boost::source(*ei,G);
                typename Representation::vertex_descriptor v=boost::target(*ei,G);

                // remove the edge:
                boost::remove_edge(*ei, G);

                // and remove the vertices if they're unconnected and not reserved:
                if(!G[u].reserved && ((boost::in_degree(u,G) + boost::out_degree(u,G)) == 0)) {
                    boost::remove_vertex(u,G);
                }
                
                if(!G[v].reserved && ((boost::in_degree(v,G) + boost::out_degree(v,G)) == 0)) {
                    boost::remove_vertex(v,G);
                }
            }
            
        } // detail

        
        /*! Direct graph mutation operator.
         
         Each possible mutation type (vertex & edge deletions, insertions, changes)
         must be defined for a direct graph representation.
         */
        struct graph_mutation {
            
            template <typename Representation, typename EA>
            void operator()(Representation& G, EA& ea) {
                using namespace boost;
                // Representation is assumed to support a boost::adjacency_list compatible interface,
                // specifically: boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS...>
                
                // add an edge to the graph:
                if(get<GRAPH_ADD_EDGE_P>(ea)) {
                    detail::add_edge(G,ea);
                }

                // remove an edge from the graph:
                if(ea.rng().p(get<GRAPH_REMOVE_EDGE_P>(ea))) {
                    detail::remove_edge(G,ea);
                }
                
                // mutate a random edge:
                if(ea.rng().p(get<GRAPH_MUTATE_EDGE_P>(ea))) {
                    typename Representation::edge_iterator ei,ei_end;
                    boost::tie(ei,ei_end) = boost::edges(G);
                    ei = ea.rng().choice(ei,ei_end);
                    G[*ei].mutate(ea);
                }
                
                // mutate a random vertex:
                if(ea.rng().p(get<GRAPH_MUTATE_VERTEX_P>(ea))) {
                    G[boost::vertex(ea.rng()(boost::num_vertices(G)),G)].mutate(ea);
                }
            }
        };

    } // mutation
    
    namespace ancestors {
        
        /*! Generates a random graph-based individual.
         */
        struct random_graph {
            template <typename EA>
            typename EA::representation_type operator()(EA& ea) {
                typename EA::representation_type G(get<GRAPH_RESERVED_VERTICES>(ea));
                
                for(std::size_t i=0; i<boost::num_vertices(G); ++i) {
                    G[boost::vertex(i,G)].reserved=true;
                }
                
                for(int i=0; i<get<GRAPH_INITIAL_EDGES>(ea); ++i) {
                    mutation::detail::add_edge(G,ea);
                }

                return G;
            }
        };
        
    } // ancestors

    namespace detail {
        struct null_type { };
    }
    
    /*! Abstract type for vertices.
     */
    template <typename Vertex=detail::null_type>
    struct abstract_vertex : Vertex {
        //! Constructor.
        abstract_vertex() : Vertex(), reserved(false) {
        }

        //! Mutate this vertex.
        template <typename EA>
        void mutate(EA& ea) {
        }
        
        bool reserved; //!< If true, this vertex will not be removed during mutation.
    };
    
    
    /*! Abstract type for edges.
     */
    template <typename Edge=detail::null_type>
    struct abstract_edge : Edge {
        //! Constructor.
        abstract_edge() : Edge() {
        }
        
        //! Mutate this edge.
        template <typename EA>
        void mutate(EA& ea) {
        }
    };
    
} // ea

#endif
