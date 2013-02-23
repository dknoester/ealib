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
#include <vector>

#include <ea/meta_data.h>

LIBEA_MD_DECL(GRAPH_MIN_SIZE, "graph.min_size", int);
LIBEA_MD_DECL(GRAPH_VERTEX_EVENT_P, "graph.vertex.event.p", double);
LIBEA_MD_DECL(GRAPH_VERTEX_ADDITION_P, "graph.vertex.addition.p", double);
LIBEA_MD_DECL(GRAPH_EDGE_EVENT_P, "graph.edge.event.p", double);
LIBEA_MD_DECL(GRAPH_EDGE_ADDITION_P, "graph.edge.addition.p", double);
LIBEA_MD_DECL(GRAPH_DUPLICATE_EVENT_P, "graph.duplicate.event.p", double);
LIBEA_MD_DECL(GRAPH_DUPLICATE_VERTEX_P, "graph.duplicate.vertex.p", double);
LIBEA_MD_DECL(GRAPH_MUTATION_EVENT_P, "graph.mutation.event.p", double);
LIBEA_MD_DECL(GRAPH_MUTATION_VERTEX_P, "graph.mutation.vertex.p", double);
LIBEA_MD_DECL(RANDOM_GRAPH_N, "random_graph.n", int);

namespace ea {
    namespace mutation {
        namespace detail {
            
            //! Add a vertex.
            template <typename Representation, typename EA>
            typename Representation::vertex_descriptor add_vertex(Representation& G, EA& ea) {
                return boost::add_vertex(G);
            }
            
            //! Remove a randomly selected vertex.
            template <typename Representation, typename EA>
            void remove_vertex(Representation& G, EA& ea) {
                typename Representation::vertex_descriptor u=boost::vertex(ea.rng()(boost::num_vertices(G)),G);
                boost::clear_vertex(u,G);
                boost::remove_vertex(u,G);
            }
            
            //! Add an edge between two distinct randomly selected vertices.
            template <typename Representation, typename EA>
            typename Representation::edge_descriptor add_edge(Representation& G, EA& ea) {
                std::size_t un,vn;
                boost::tie(un,vn) = ea.rng().choose_two(0, static_cast<int>(boost::num_vertices(G)));
                typename Representation::vertex_descriptor u=boost::vertex(un,G);
                typename Representation::vertex_descriptor v=boost::vertex(vn,G);
                return boost::add_edge(u,v,G).first;
            }
            
            //! Remove a randomly selected edge.
            template <typename Representation, typename EA>
            void remove_edge(Representation& G, EA& ea) {
                typename Representation::edge_iterator ei,ei_end;
                boost::tie(ei,ei_end) = boost::edges(G);
                boost::remove_edge(*ea.rng().choice(ei,ei_end), G);
            }
            
            //! Copy E_in(u) -> E_in(v).
            template <typename VertexDescriptor, typename Graph>
            void copy_in_edges(VertexDescriptor u, VertexDescriptor v, Graph& G) {
                typedef std::vector<std::pair<typename Graph::vertex_descriptor,
                typename Graph::edge_descriptor> > ve_list;
                ve_list adjacent;
                
                typename Graph::inv_adjacency_iterator iai,iai_end;
                for(boost::tie(iai,iai_end)=boost::inv_adjacent_vertices(u,G); iai!=iai_end; ++iai) {
                    adjacent.push_back(std::make_pair(*iai, boost::edge(*iai,u,G).first));
                }
                for(typename ve_list::iterator i=adjacent.begin(); i!=adjacent.end(); ++i) {
                    boost::add_edge(i->first, v, G[i->second], G);
                }
            }
            
            //! Copy E_out(u) -> E_out(v)
            template <typename VertexDescriptor, typename Graph>
            void copy_out_edges(VertexDescriptor u, VertexDescriptor v, Graph& G) {
                typedef std::vector<std::pair<typename Graph::vertex_descriptor,
                typename Graph::edge_descriptor> > ve_list;
                ve_list adjacent;
                
                typename Graph::adjacency_iterator ai,ai_end;
                for(boost::tie(ai,ai_end)=boost::adjacent_vertices(u,G); ai!=ai_end; ++ai) {
                    adjacent.push_back(std::make_pair(*ai, boost::edge(u,*ai,G).first));
                }
                for(typename ve_list::iterator i=adjacent.begin(); i!=adjacent.end(); ++i) {
                    boost::add_edge(v, i->first, G[i->second], G);
                }
            }
            
            //! Duplicate a randomly selected vertex.
            template <typename Representation, typename EA>
            void duplicate_vertex(Representation& G, EA& ea) {
                typename Representation::vertex_descriptor u=boost::vertex(ea.rng()(boost::num_vertices(G)),G);
                typename Representation::vertex_descriptor v=boost::add_vertex(G[u],G);
                copy_in_edges(u,v,G);
                copy_out_edges(u,v,G);
            }
            
            //! Merge two randomly selected vertices.
            template <typename Representation, typename EA>
            void merge_vertices(Representation& G, EA& ea) {
                std::size_t un,vn;
                boost::tie(un,vn) = ea.rng().choose_two(0, static_cast<int>(boost::num_vertices(G)));
                typename Representation::vertex_descriptor u=boost::vertex(un,G);
                typename Representation::vertex_descriptor v=boost::vertex(vn,G);
                
                // edges incident to v are copied to u; v is then cleared and erased.
                copy_in_edges(v,u,G);
                copy_out_edges(v,u,G);
                boost::clear_vertex(v,G);
                boost::remove_vertex(v,G);
            }
            
            //! Mutate a randomonly selected vertex.
            template <typename Representation, typename EA>
            void mutate_vertex(Representation& G, EA& ea) {
                G[boost::vertex(ea.rng()(boost::num_vertices(G)),G)].mutate(ea);
            }
            
            //! Mutate a randomonly selected edge.
            template <typename Representation, typename EA>
            void mutate_edge(Representation& G, EA& ea) {
                typename Representation::edge_iterator ei,ei_end;
                boost::tie(ei,ei_end) = boost::edges(G);
                G[*ea.rng().choice(ei,ei_end)].mutate(ea);
            }
        } // detail
        
        
        /*! Graph mutations, based on general growth operations
         
         \warning The graph mutation types described here allow self-loops and
         do not explicitly prevent parallel edges (though careful selection of
         the underlying graph type can do so).
         
         Note that there is a minimum size on graphs that is respected.
         */
        struct graph_mutation {
            template <typename Representation, typename EA>
            void operator()(Representation& G, EA& ea) {
                if(ea.rng().p(get<GRAPH_VERTEX_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_VERTEX_ADDITION_P>(ea))) {
                        detail::add_vertex(G,ea);
                    } else if(boost::num_vertices(G) > get<GRAPH_MIN_SIZE>(ea)) {
                        detail::remove_vertex(G,ea);
                    }
                }
                
                if(ea.rng().p(get<GRAPH_EDGE_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_EDGE_ADDITION_P>(ea))) {
                        detail::add_edge(G,ea);
                    } else {
                        detail::remove_edge(G,ea);
                    }
                }
                
                if(ea.rng().p(get<GRAPH_DUPLICATE_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_DUPLICATE_VERTEX_P>(ea))) {
                        detail::duplicate_vertex(G,ea);
                    } else if(boost::num_vertices(G) > get<GRAPH_MIN_SIZE>(ea)) {
                        detail::merge_vertices(G,ea);
                    }
                }
                
                if(ea.rng().p(get<GRAPH_MUTATION_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_MUTATION_VERTEX_P>(ea))) {
                        detail::mutate_vertex(G,ea);
                    } else {
                        detail::mutate_edge(G,ea);
                    }
                }
            }
        };
    } // mutation
    
    namespace ancestors {
        
        /*! Generates a random graph-based individual based on a minimum graph size
         and a configurable number of calls to the graph mutation operator.
         */
        struct random_graph {
            template <typename EA>
            void randomize(typename EA::representation_type& G, EA& ea) {
                mutation::graph_mutation gm;
                for(int i=0; i<get<RANDOM_GRAPH_N>(ea); ++i) {
                    gm(G,ea);
                }
            }
            
            template <typename EA>
            typename EA::representation_type operator()(EA& ea) {
                typename EA::representation_type G(get<GRAPH_MIN_SIZE>(ea));
                randomize(G,ea);
                return G;
            }
            
        };
    } // ancestors
    
    namespace detail {
        struct null_type { };
    }
    
    //! Abstract type for vertices.
    template <typename Vertex=detail::null_type>
    struct abstract_vertex : Vertex {
        //! Constructor.
        abstract_vertex() : Vertex() {
        }
        
        //! Mutate this vertex.
        template <typename EA>
        void mutate(EA& ea) {
        }
    };
    
    //! Abstract type for edges.
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
