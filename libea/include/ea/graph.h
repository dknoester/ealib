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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <vector>
#include <sstream>
#include <ea/algorithm.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/rng.h>

LIBEA_MD_DECL(GRAPH_EVENTS_N, "graph.events.n", int);
LIBEA_MD_DECL(GRAPH_VERTEX_EVENT_P, "graph.vertex.event.p", double);
LIBEA_MD_DECL(GRAPH_VERTEX_ADDITION_P, "graph.vertex.addition.p", double);
LIBEA_MD_DECL(GRAPH_EDGE_EVENT_P, "graph.edge.event.p", double);
LIBEA_MD_DECL(GRAPH_EDGE_ADDITION_P, "graph.edge.addition.p", double);
LIBEA_MD_DECL(GRAPH_DUPLICATE_EVENT_P, "graph.duplicate.event.p", double);
LIBEA_MD_DECL(GRAPH_DUPLICATE_VERTEX_P, "graph.duplicate.vertex.p", double);
LIBEA_MD_DECL(GRAPH_MUTATION_EVENT_P, "graph.mutation.event.p", double);
LIBEA_MD_DECL(GRAPH_MUTATION_VERTEX_P, "graph.mutation.vertex.p", double);

namespace ealib {
    namespace graph {
        //! These are the different graph operations that are allowed.
        namespace graph_operation {
            enum flag { remove, merge, duplicate, source, target, mutate };
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

        //! Adaptor to add color to vertices.
        struct colored_vertex {
            typedef int color_type;
            colored_vertex() : color(0) { }
            color_type color; //!< Color of this vertex, used to assign modularity.
        };
        
        //! Mix-in vertex type for graphs that are grown.
        struct grown_vertex : colored_vertex {
            //! Default constructor.
            grown_vertex() { }

            //! Returns true if the given graph operation is allowed.
            bool allows(graph::graph_operation::flag m) {
                return true;
            }
        };

        //! Mix-in vertex type for graphs that are grown.
        struct grown_edge {
            //! Returns true if the given graph operation is allowed.
            bool allows(graph::graph_operation::flag m) {
                return true;
            }
        };
        
        //! Mix-in for mutable vertices.
        struct mutable_vertex : grown_vertex {
            //! Constructor.
            mutable_vertex() {
            }
            
            //! Mutate this vertex.
            template <typename EA>
            void mutate(EA& ea) {
            }
        };
        
        //! Mix-in for mutable edges.
        struct mutable_edge : grown_edge {
            //! Constructor.
            mutable_edge() {
            }
            
            //! Mutate this edge.
            template <typename EA>
            void mutate(EA& ea) {
            }
        };

        /*! Contains information needed to grow graphs.
         */
        struct growth_descriptor {
            typedef std::vector<double> pr_sequence_type;
            typedef boost::numeric::ublas::matrix<double> assortativity_matrix_type;
            
            //! Default constructor.
            growth_descriptor() : Pe(3,0.0), Pc(3,0.0), Pm(1,1.0), M(1,1) {
                M(0,0) = 1.0;
            }
            
            growth_descriptor(double pv, double pe, double pd, double p, double q, double r)
            : Pe(3,0.0), Pc(3,0.0), Pm(1,1.0), M(1,1) {
                Pe[growth::P_V] = pv;
                Pe[growth::P_E] = pe;
                Pe[growth::P_D] = pd;
                Pc[conditional::p] = p;
                Pc[conditional::q] = q;
                Pc[conditional::r] = r;
                M(0,0) = 1.0;
            }
            
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & BOOST_SERIALIZATION_NVP(Pe);
                ar & BOOST_SERIALIZATION_NVP(Pc);
                ar & BOOST_SERIALIZATION_NVP(Pm);
                ar & BOOST_SERIALIZATION_NVP(M);
            }
            
            pr_sequence_type Pe; //!< Event probabilities.
            pr_sequence_type Pc; //!< Conditional probabilities.
            pr_sequence_type Pm; //!< Module probabilities.
            assortativity_matrix_type M; //!< Module assortativity matrix.
        };

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

        /*
         The methods below here are "best-effort" -- that is, they make an attempt
         to perform the requested operation, but may not, either because the graph
         does not allow it (e.g., no edges to remove), or due to stochastic effects
         (e.g., we didn't select vertices that may be merged).
         */
        
        //! Add a vertex.
        template <typename Graph, typename RNG>
        typename Graph::vertex_descriptor add_vertex(Graph& G, RNG& rng, const growth_descriptor& D=growth_descriptor()) {
            typename Graph::vertex_descriptor v=boost::add_vertex(G);
            G[v].color = algorithm::roulette_wheel(rng.p(),D.Pm.begin(), D.Pm.end()).first;
            return v;
        }
        
        //! Remove a randomly selected vertex.
        template <typename Graph, typename RNG>
        void remove_vertex(Graph& G, RNG& rng, const growth_descriptor& D=growth_descriptor()) {
            if(boost::num_vertices(G) == 0) {
                return;
            }
            
            typename Graph::vertex_descriptor u=boost::vertex(rng(boost::num_vertices(G)),G);
            
            if(G[u].allows(graph_operation::remove)) {
                boost::clear_vertex(u,G);
                boost::remove_vertex(u,G);
            }
        }
        
        //! Add an edge between two distinct randomly selected vertices.
        template <typename Graph, typename RNG>
        std::pair<typename Graph::edge_descriptor,bool> add_edge(Graph& G, RNG& rng, const growth_descriptor& D=growth_descriptor()) {
            if(boost::num_vertices(G) <= 1) {
                return std::make_pair(typename Graph::edge_descriptor(),false);
            }

            for(std::size_t i=0; i<1000; ++i) {
                std::size_t un,vn;
                boost::tie(un,vn) = rng.choose_two_ns(0, static_cast<int>(boost::num_vertices(G)));
                typename Graph::vertex_descriptor u=boost::vertex(un,G);
                typename Graph::vertex_descriptor v=boost::vertex(vn,G);
                
                if(G[u].allows(graph_operation::source)
                   && G[v].allows(graph_operation::target)
                   && rng.p(D.M(G[u].color,G[v].color))) {
                    return boost::add_edge(u,v,G);
                }
            }

            return std::make_pair(typename Graph::edge_descriptor(),false);
        }
        
        //! Remove a randomly selected edge.
        template <typename Graph, typename RNG>
        void remove_edge(Graph& G, RNG& rng, const growth_descriptor& D=growth_descriptor()) {
            if(boost::num_edges(G) == 0) {
                return;
            }
            
            typename Graph::edge_iterator ei,ei_end;
            boost::tie(ei,ei_end) = boost::edges(G);
            ei = rng.choice(ei,ei_end);
            
            if(G[boost::source(*ei,G)].allows(graph_operation::source) && G[boost::target(*ei,G)].allows(graph_operation::target)) {
                boost::remove_edge(*ei,G);
            }
        }
        
        //! Duplicate a randomly selected vertex.
        template <typename Graph, typename RNG>
        void duplicate_vertex(Graph& G, RNG& rng, const growth_descriptor& D=growth_descriptor()) {
            if(boost::num_vertices(G) == 0) {
                return;
            }
            
            typename Graph::vertex_descriptor u=boost::vertex(rng(boost::num_vertices(G)),G);
            
            if(G[u].allows(graph_operation::duplicate)) {
                typename Graph::vertex_descriptor v=boost::add_vertex(G);
                G[v].color = G[u].color;
                copy_in_edges(u,v,G);
                copy_out_edges(u,v,G);
            }
        }
        
        //! Merge two randomly selected vertices.
        template <typename Graph, typename RNG>
        void merge_vertices(Graph& G, RNG& rng, const growth_descriptor& D=growth_descriptor()) {
            // merge
            if(boost::num_vertices(G) <= 1) {
                return;
            }
            
            std::size_t un,vn;
            boost::tie(un,vn) = rng.choose_two_ns(0, static_cast<int>(boost::num_vertices(G)));
            typename Graph::vertex_descriptor u=boost::vertex(un,G);
            typename Graph::vertex_descriptor v=boost::vertex(vn,G);
            
            if(G[u].allows(graph_operation::merge) && G[v].allows(graph_operation::merge)) {
                copy_in_edges(v,u,G);
                copy_out_edges(v,u,G);
                boost::clear_vertex(v,G);
                boost::remove_vertex(v,G);
            }
        }
   
        /*! Perform n growth events on graph G via the given growth descriptor.
         
         Vertices in G must have an internal "color" property.  Inheriting from
         mutable_vertex (see above) will suffice.  All vertices initially in G
         are assumed to have a valid color (0 is acceptable).
         
         Vertices and edges must both provide an "allows" operator.  See mutable_vertex
         and mutable_edge above.
         */
        template <typename Graph, typename RNG>
        void grow_network(Graph& G, int n, growth_descriptor& D, RNG& rng) {
            typedef typename Graph::vertex_descriptor vertex_descriptor;
            typedef typename Graph::edge_descriptor edge_descriptor;
            
            // sanity...
            assert(D.Pm.size() == D.M.size1());
            assert(D.Pm.size() == D.M.size2());
            assert(D.Pe.size() == 3);
            assert(D.Pc.size() == 3);

            // normalize event and module probabilities to 1.0:
            algorithm::normalize(D.Pe.begin(), D.Pe.end(), 1.0);
            algorithm::normalize(D.Pm.begin(), D.Pm.end(), 1.0);

            for( ; n>0; --n) {
                switch(algorithm::roulette_wheel(rng.p(), D.Pe.begin(), D.Pe.end()).first) {
                    case 0: {
                        if(rng.p(D.Pc[conditional::p])) {
                            add_vertex(G,rng,D);
                        } else {
                            remove_vertex(G,rng,D);
                        }
                        break;
                    }
                    case 1: {
                        if(rng.p(D.Pc[conditional::q])) {
                            add_edge(G,rng,D);
                        } else {
                            remove_edge(G,rng,D);
                        }
                        break;
                    }
                    case 2: {
                        if(rng.p(D.Pc[conditional::r])) {
                            duplicate_vertex(G,rng,D);
                        } else {
                            merge_vertices(G,rng,D);
                        }
                        break;
                    }
                    default: {
                        assert(false);
                    }
                }
            }
        }

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

        //! Mutate a randomonly selected vertex.
        template <typename Representation, typename EA>
        void mutate_vertex(Representation& G, EA& ea) {
            if(boost::num_vertices(G) == 0) {
                return;
            }
            
            typename Representation::vertex_descriptor u=boost::vertex(ea.rng()(boost::num_vertices(G)),G);
            if(G[u].allows(graph::graph_operation::mutate)) {
                G[u].mutate(ea);
            }
        }
        
        //! Mutate a randomonly selected edge.
        template <typename Representation, typename EA>
        void mutate_edge(Representation& G, EA& ea) {
            if(boost::num_edges(G) == 0) {
                return;
            }
            
            typename Representation::edge_iterator ei,ei_end;
            boost::tie(ei,ei_end) = boost::edges(G);
            ei = ea.rng().choice(ei,ei_end);
            if(G[*ei].allows(graph::graph_operation::mutate)) {
                G[*ei].mutate(ea);
            }
        }
        
        
        struct graph_rep_mutator {
            template <typename EA>
            void operator()(typename EA::representation_type& G, EA& ea) {
                graph::growth_descriptor D(get<GRAPH_VERTEX_EVENT_P>(ea),
                                           get<GRAPH_EDGE_EVENT_P>(ea),
                                           get<GRAPH_DUPLICATE_EVENT_P>(ea),
                                           get<GRAPH_VERTEX_ADDITION_P>(ea),
                                           get<GRAPH_EDGE_ADDITION_P>(ea),
                                           get<GRAPH_DUPLICATE_VERTEX_P>(ea));
                
                grow_network(G, 1, D, ea.rng());
                
                if(ea.rng().p(get<GRAPH_MUTATION_EVENT_P>(ea))) {
                    if(ea.rng().p(get<GRAPH_MUTATION_VERTEX_P>(ea))) {
                        mutate_vertex(G,ea);
                    } else {
                        mutate_edge(G,ea);
                    }
                }
            }
        };
        
        
        /*! Mutate a graph.
         
         This mutation operator performs one graph growth event, and may then
         attempt to mutate either a vertex or edge.
         
         \warning The graph mutation types described here allow self-loops and
         do not explicitly prevent parallel edges (though careful selection of
         the underlying graph type can do so).
         */
        struct graph_mutator {
            template <typename EA>
            void operator()(typename EA::individual_type& ind, EA& ea) {
                _m(ind.repr(), ea);
            }
            graph_rep_mutator _m;
        };

        /*! Mutate a graph growth descriptor.
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
                typename EA::representation_type G;
                mutation::graph_rep_mutator gm;
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
