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
#include <boost/numeric/ublas/matrix.hpp>
#include <vector>
#include <sstream>
#include <ea/algorithm.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/rng.h>

LIBEA_MD_DECL(GRAPH_MIN_SIZE, "graph.min_size", int);
LIBEA_MD_DECL(GRAPH_EVENTS_N, "graph.events.n", int);
LIBEA_MD_DECL(GRAPH_VERTEX_EVENT_P, "graph.vertex.event.p", double);
LIBEA_MD_DECL(GRAPH_VERTEX_ADDITION_P, "graph.vertex.addition.p", double);
LIBEA_MD_DECL(GRAPH_EDGE_EVENT_P, "graph.edge.event.p", double);
LIBEA_MD_DECL(GRAPH_EDGE_ADDITION_P, "graph.edge.addition.p", double);
LIBEA_MD_DECL(GRAPH_DUPLICATE_EVENT_P, "graph.duplicate.event.p", double);
LIBEA_MD_DECL(GRAPH_DUPLICATE_VERTEX_P, "graph.duplicate.vertex.p", double);
LIBEA_MD_DECL(GRAPH_MUTATION_EVENT_P, "graph.mutation.event.p", double);
LIBEA_MD_DECL(GRAPH_MUTATION_VERTEX_P, "graph.mutation.vertex.p", double);
LIBEA_MD_DECL(GRAPH_PATH_EVENT_P, "graph.path.event.p", double);
LIBEA_MD_DECL(GRAPH_PATH_MAX_LENGTH, "graph.path.max_length", int);

namespace ea {
    namespace graph {
        //! These are the different graph operations that are allowed.
        namespace graph_operation {
            enum flag { remove, merge, duplicate, source, target };
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
        
        //! Add a vertex.
        template <typename Graph, typename EA>
        typename Graph::vertex_descriptor add_vertex(Graph& G, EA& ea) {
            return boost::add_vertex(G);
        }
        
        //! Remove a randomly selected vertex.
        template <typename Graph, typename EA>
        void remove_vertex(Graph& G, EA& ea) {
            if(boost::num_vertices(G) <= get<GRAPH_MIN_SIZE>(ea)) {
                return;
            }
            
            typename Graph::vertex_descriptor u=boost::vertex(ea.rng()(boost::num_vertices(G)),G);
            
            if(G[u].allows(graph_operation::remove)) {
                boost::clear_vertex(u,G);
                boost::remove_vertex(u,G);
            }
        }
        
        //! Add an edge between two distinct randomly selected vertices.
        template <typename Graph, typename EA>
        std::pair<typename Graph::edge_descriptor,bool> add_edge(Graph& G, EA& ea) {
            if(boost::num_vertices(G) <= 1) {
                return std::make_pair(typename Graph::edge_descriptor(),false);
            }
            
            std::size_t un,vn;
            boost::tie(un,vn) = ea.rng().choose_two_ns(0, static_cast<int>(boost::num_vertices(G)));
            typename Graph::vertex_descriptor u=boost::vertex(un,G);
            typename Graph::vertex_descriptor v=boost::vertex(vn,G);
            
            if(G[u].allows(graph_operation::source) && G[v].allows(graph_operation::target)) {
                return boost::add_edge(u,v,G);
            } else {
                return std::make_pair(typename Graph::edge_descriptor(),false);
            }
        }
        
        //! Remove a randomly selected edge.
        template <typename Graph, typename EA>
        void remove_edge(Graph& G, EA& ea) {
            if(boost::num_edges(G) == 0) {
                return;
            }
            
            typename Graph::edge_iterator ei,ei_end;
            boost::tie(ei,ei_end) = boost::edges(G);
            ei = ea.rng().choice(ei,ei_end);
            
            if(G[boost::source(*ei,G)].allows(graph_operation::source) && G[boost::target(*ei,G)].allows(graph_operation::target)) {
                boost::remove_edge(*ei,G);
            }
        }
        
        //! Duplicate a randomly selected vertex.
        template <typename Graph, typename EA>
        void duplicate_vertex(Graph& G, EA& ea) {
            if(boost::num_vertices(G) == 0) {
                return;
            }
            
            typename Graph::vertex_descriptor u=boost::vertex(ea.rng()(boost::num_vertices(G)),G);
            
            if(G[u].allows(graph_operation::duplicate)) {
                typename Graph::vertex_descriptor v=boost::add_vertex(G);
                graph::copy_in_edges(u,v,G);
                graph::copy_out_edges(u,v,G);
            }
        }
        
        //! Merge two randomly selected vertices.
        template <typename Graph, typename EA>
        void merge_vertices(Graph& G, EA& ea) {
            if(boost::num_vertices(G) <= get<GRAPH_MIN_SIZE>(ea)) {
                return;
            }
            
            std::size_t un,vn;
            boost::tie(un,vn) = ea.rng().choose_two_ns(0, static_cast<int>(boost::num_vertices(G)));
            typename Graph::vertex_descriptor u=boost::vertex(un,G);
            typename Graph::vertex_descriptor v=boost::vertex(vn,G);
            
            if(G[u].allows(graph_operation::merge) && G[v].allows(graph_operation::merge)) {
                // edges incident to v are copied to u; v is then cleared and erased.
                graph::copy_in_edges(v,u,G);
                graph::copy_out_edges(v,u,G);
                boost::clear_vertex(v,G);
                boost::remove_vertex(v,G);
            }
        }
        
        /* These probabilities describe how graphs are grown:
         
         P_V is Node-event probability.
         P_E is Edge-event probability.
         P_D is Duplication-event probability.
         p is Conditional node addition probability.
         q is Conditional edge addition probability.
         r is Conditional node duplication probability.
         */
        namespace growth {
            enum probability { P_V=0, P_E, P_D };
        }
        namespace conditional {
            enum probability { p=0, q, r };
        }
        
        /*! Contains information needed to grow graphs.
         */
        struct growth_descriptor {
            typedef std::vector<double> pr_sequence_type;
            typedef boost::numeric::ublas::matrix<double> assortativity_matrix_type;

            //! Default constructor.
            growth_descriptor() : Pe(3,0.0), Pc(3,0.0), Pm(1,1.0), M(1,1) {
                M(0,0) = 1.0;
            }
            
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) const {
            }
            
            pr_sequence_type Pe; //!< Event probabilities.
            pr_sequence_type Pc; //!< Conditional probabilities.
            pr_sequence_type Pm; //!< Module probabilities.
            assortativity_matrix_type M; //!< Module assortativity matrix.
        };
        
        /*! Perform n growth events on graph G given a growth descriptor.
         
         G is guaranteed to not shrink below its initial size; all vertices initially
         in G are assumed to have a valid color (0 is acceptable).
         
         Vertices in G must have an internal "color" property; see below for an
         colored vertex adaptor.
         */
        template <typename Graph, typename RNG>
        void grow_network(Graph& G, int n, growth_descriptor& D, RNG& rng) {
            typedef typename Graph::vertex_descriptor vertex_descriptor;
            typedef typename Graph::edge_descriptor edge_descriptor;
            
            // sanity...
            assert(D.Pm.size() == D.M.size1());
            assert(D.Pm.size() == D.M.size2());
            
            // normalize event and module probabilities to 1.0:
            algorithm::normalize(D.Pe.begin(), D.Pe.end(), 1.0);
            algorithm::normalize(D.Pm.begin(), D.Pm.end(), 1.0);
            
            // G is not allowed to shrink below it's initial size:
            int minsize = boost::num_vertices(G);

            for( ; n>0; --n) {
                switch(algorithm::roulette_wheel(rng.p(), D.Pe.begin(), D.Pe.end()).first) {
                    case 0: {
                        if(rng.p(D.Pc[conditional::p])) {
                            // add node
                            vertex_descriptor v=boost::add_vertex(G);
                            G[v].color = algorithm::roulette_wheel(rng.p(),D.Pm.begin(), D.Pm.end()).first;
                        } else {
                            // remove node
                            if(boost::num_vertices(G) <= minsize) {
                                continue;
                            }
                            
                            vertex_descriptor u=boost::vertex(rng(boost::num_vertices(G)),G);
                            
                            if(G[u].allows(graph_operation::remove)) {
                                boost::clear_vertex(u,G);
                                boost::remove_vertex(u,G);
                            }
                        }
                        break;
                    }
                    case 1: {
                        if(rng.p(D.Pc[conditional::q])) {
                            // add edge
                            if(boost::num_vertices(G) <= 1) {
                                continue;
                            }
                            
                            for(std::size_t i=0; i<1000; ++i) {
                                std::size_t un,vn;
                                boost::tie(un,vn) = rng.choose_two_ns(0, static_cast<int>(boost::num_vertices(G)));
                                vertex_descriptor u=boost::vertex(un,G);
                                vertex_descriptor v=boost::vertex(vn,G);
                                
                                if(G[u].allows(graph_operation::source)
                                   && G[v].allows(graph_operation::target)
                                   && rng.p(D.M(G[u].color,G[v].color))) {
                                    boost::add_edge(u,v,G);
                                    break;
                                }
                            }
                        } else {
                            // remove edge
                            if(boost::num_edges(G) == 0) {
                                continue;
                            }
                            
                            typename Graph::edge_iterator ei,ei_end;
                            boost::tie(ei,ei_end) = boost::edges(G);
                            ei = rng.choice(ei,ei_end);
                            
                            if(G[boost::source(*ei,G)].allows(graph_operation::source) && G[boost::target(*ei,G)].allows(graph_operation::target)) {
                                boost::remove_edge(*ei,G);
                            }
                        }
                        break;
                    }
                    case 2: {
                        if(rng.p(D.Pc[conditional::r])) {
                            // duplicate
                            if(boost::num_vertices(G) == 0) {
                                continue;
                            }
                            
                            vertex_descriptor u=boost::vertex(rng(boost::num_vertices(G)),G);
                            
                            if(G[u].allows(graph_operation::duplicate)) {
                                vertex_descriptor v=boost::add_vertex(G);
                                G[v].color = G[u].color;
                                copy_in_edges(u,v,G);
                                copy_out_edges(u,v,G);
                            }
                        } else {
                            // merge
                            if(boost::num_vertices(G) <= std::max(minsize,1)) {
                                continue;
                            }
                            
                            std::size_t un,vn;
                            boost::tie(un,vn) = rng.choose_two_ns(0, static_cast<int>(boost::num_vertices(G)));
                            vertex_descriptor u=boost::vertex(un,G);
                            vertex_descriptor v=boost::vertex(vn,G);
                            
                            if(G[u].allows(graph_operation::merge) && G[v].allows(graph_operation::merge)) {
                                // edges incident to v are copied to u; v is then cleared and erased.
                                copy_in_edges(v,u,G);
                                copy_out_edges(v,u,G);
                                boost::clear_vertex(v,G);
                                boost::remove_vertex(v,G);
                            }
                        }
                        break;
                    }
                    default: {
                        assert(false);
                    }
                }
            }
        }

        namespace detail {
            //! Null-type; used as a type placeholder for "plain" graphs.
            struct null_type { };
        }
        
        /*! Vertex adaptor that add a "color" property to the given vertex type.
         */
        template <typename Vertex=detail::null_type>
        struct colored_vertex : Vertex {
            typedef int color_type;
            colored_vertex() : color(0) { }
            color_type color;
        };
        
        //! Convenience method to convert a Graph to a graphviz-compatible string (boost::write_graphviz doesn't work with EALib).
        template <typename Graph>
        std::string graph2string(Graph& G) {
            std::ostringstream out;
            
            out << "digraph {" << std::endl << "edge [ arrowsize=0.75 ];" << std::endl;
            
            for(std::size_t i=0; i<boost::num_vertices(G); ++i) {
                out << boost::vertex(i,G) << ";" << std::endl;
            }
            
            typename Graph::edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(G); ei!=ei_end; ++ei) {
                out << boost::source(*ei,G) << "->" << boost::target(*ei,G) << ";" << std::endl;
            }
            
            out << "}" << std::endl;
            return out.str();
        }
    } // graph
    
    namespace mutation {
        /*! Mutable vertex adaptor.
         */
        template <typename Vertex=graph::detail::null_type>
        struct mutable_vertex : Vertex {
            //! Constructor.
            mutable_vertex() : Vertex() {
            }
            
            //! Returns true if the given graph mutation is allowed.
            bool allows(graph::graph_operation::flag m) {
                return true;
            }
            
            //! Mutate this vertex.
            template <typename EA>
            void mutate(EA& ea) {
            }
        };
        
        /*! Mutable edge adaptor.
         */
        template <typename Edge=graph::detail::null_type>
        struct mutable_edge : Edge {
            //! Constructor.
            mutable_edge() : Edge() {
            }
            
            //! Mutate this edge.
            template <typename EA>
            void mutate(EA& ea) {
            }
        };

        //! Mutate a randomonly selected vertex.
        template <typename Representation, typename EA>
        void mutate_vertex(Representation& G, EA& ea) {
            if(boost::num_vertices(G) == 0) {
                return;
            }
            
            typename Representation::vertex_descriptor u=boost::vertex(ea.rng()(boost::num_vertices(G)),G);
            G[u].mutate(ea);
        }
        
        //! Mutate a randomonly selected edge.
        template <typename Representation, typename EA>
        void mutate_edge(Representation& G, EA& ea) {
            if(boost::num_edges(G) == 0) {
                return;
            }
            
            typename Representation::edge_iterator ei,ei_end;
            boost::tie(ei,ei_end) = boost::edges(G);
            G[*ea.rng().choice(ei,ei_end)].mutate(ea);
        }
        
        /*! Graph mutations, based on general growth operations
         
         \warning The graph mutation types described here allow self-loops and
         do not explicitly prevent parallel edges (though careful selection of
         the underlying graph type can do so).
         
         Note that there is a minimum size on graphs that is respected.
         */
        struct graph_mutator {
            template <typename Representation, typename EA>
            void operator()(Representation& G, EA& ea) {
                if(ea.rng().p(get<GRAPH_VERTEX_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_VERTEX_ADDITION_P>(ea))) {
                        graph::add_vertex(G,ea);
                    } else {
                        graph::remove_vertex(G,ea);
                    }
                }
                
                if(ea.rng().p(get<GRAPH_EDGE_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_EDGE_ADDITION_P>(ea))) {
                        graph::add_edge(G,ea);
                    } else {
                        graph::remove_edge(G,ea);
                    }
                }
                
                if(ea.rng().p(get<GRAPH_DUPLICATE_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_DUPLICATE_VERTEX_P>(ea))) {
                        graph::duplicate_vertex(G,ea);
                    } else {
                        graph::merge_vertices(G,ea);
                    }
                }
                
                if(ea.rng().p(get<GRAPH_MUTATION_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_MUTATION_VERTEX_P>(ea))) {
                        mutate_vertex(G,ea);
                    } else {
                        mutate_edge(G,ea);
                    }
                }
            }
        };

        /*! Mutation operator for growth descriptors.
         */
        struct growth_descriptor_mutator {
            typedef mutation::per_site<mutation::uniform_real> sequence_mutator_type;
            
            template <typename Representation, typename EA>
            void operator()(Representation& repr, EA& ea) {
                sequence_mutator_type sm;
                
                sm(repr.Pe, ea);
                sm(repr.Pc, ea);
                sm(repr.Pm, ea);
                sm(repr.M.data(), ea);
                
                // P of changing # of modules is persite / #modules
                if(ea.rng().p(get<MUTATION_PER_SITE_P>(ea)/static_cast<double>(repr.Pm.size()))) {
                    if(ea.rng().bit()) {
                        // increase
                        repr.Pm.push_back(ea.rng().p());
                        repr.M.resize(repr.M.size1()+1, repr.M.size2()+1, true);
                        for(std::size_t i=0; i<repr.M.size1(); ++i) {
                            repr.M(i,repr.M.size2()-1) = ea.rng().p();
                        }
                        for(std::size_t j=0; j<repr.M.size2(); ++j) {
                            repr.M(repr.M.size1()-1,j) = ea.rng().p();
                        }
                    } else if(repr.Pm.size() > 1) {
                        // decrease
                        repr.Pm.pop_back();
                        repr.M.resize(repr.M.size1()-1, repr.M.size2()-1, true);
                    }
                }
            }
        };
    } // mutation
    
    namespace ancestors {
        /*! Generates a random graph representation based on calling the graph
         mutation operator GRAPH_EVENTS_N times.
         */
        struct random_graph {
            template <typename EA>
            typename EA::representation_type operator()(EA& ea) {
                typename EA::representation_type G(get<GRAPH_MIN_SIZE>(ea));
                mutation::graph_mutator gm;
                for(int i=0; i<get<GRAPH_EVENTS_N>(ea); ++i) {
                    gm(G,ea);
                }
                return G;
            }
        };

        /*! Generates a random growth descriptor for a single module graph.
         */
        struct random_growth_descriptor {
            template <typename EA>
            typename EA::representation_type operator()(EA& ea) {
                typename EA::representation_type repr;
                
                std::generate(repr.Pe.begin(), repr.Pe.end(), probability_generator<typename EA::rng_type>(ea.rng()));
                std::generate(repr.Pc.begin(), repr.Pc.end(), probability_generator<typename EA::rng_type>(ea.rng()));

                // we rely on default construction of the growth descriptor to be sane;
                // ie, one module, 100% likelihood of edges connecting.
                return repr;
            }
        };
    } // ancestors
} // ea

#endif
