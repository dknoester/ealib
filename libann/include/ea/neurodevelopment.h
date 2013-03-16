/* neurodevelopment.h
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
#ifndef _EA_NEURODEVELOPMENT_H_
#define _EA_NEURODEVELOPMENT_H_

#include <vector>
#include <ann/abstract_neuron.h>
#include <ea/algorithm.h>
#include <ea/graph.h>
#include <ea/meta_data.h>

LIBEA_MD_DECL(DEV_VERTICES_N, "development.vertices.n", int);

namespace ea {
    
    /* The following code develops a graph from a "developmental template," which 
     is itself a graph that defines a high-level topology.
     
     It is based on the idea that we can evolve the gross topology of a neural network,
     but that the connectivity of individual neurons is stochastic.  Edge weights
     in the neural network are assumed to be controlled by a learning process.
     
     There are three pieces to be aware of:
     - The developmental template is a graph D(M,L), where M are vertices and L are
     edges.  Each m \in M is termed a "module".
     - The graph that is developed from D(M,L) is G(V,E), where V are vertices and E
     are edges.  Each v \in V is a computational unit, e.g., an artificial neuron.
     Each e \in E are connections between computational units.
     - A function \delta() is responsible for producing G(V,E) from D(M,L):
     G = \delta(n,D)
     where n is the number of resultant vertices in G, ||V||.
     
     Importantly, evolution **ONLY** operates on D(M,L), using the graph mutation
     operations provided in ea/graph.h.
     */
    
    /*! Vertex in a developmental network.
     
     Each vertex corresponds to a module m \in M of D(M,L).
     */
    struct developmental_vertex : graph::colored_vertex, ann::neuron_flags {
        //! Returns true if the given graph mutation is allowed.
        bool allows(graph::graph_operation::flag m) {
            using namespace graph;
            using namespace ann;
            switch(m) {
                case graph_operation::remove: {
                    return !getf(neuron::reserved);
                }
                case graph_operation::merge: {
                    return !getf(neuron::reserved);
                }
                case graph_operation::duplicate: {
                    return !getf(neuron::reserved);
                }
                case graph_operation::source: {
                    return getf(neuron::input) || getf(neuron::output) || getf(neuron::hidden);
                }
                case graph_operation::target: {
                    return getf(neuron::hidden) || getf(neuron::output);
                }
                case graph_operation::mutate: {
                    return true;
                }
            }
        }

        //! Mutate this vertex.
        template <typename EA>
        void mutate(EA& ea) {
        }

        double weight; //!< Relative weight of this vertex.
        double degree_mean; //!< Mean degree of neurons belonging to this vertex.
        double degree_var; //!< Variance of the degree of neurons belonging to this vertex.

        // while per-module learning rates are appealing, best leave this out for now:
        //double learning_rate; //!< Learning rate for neurons belonging to this vertex.
        //double decay_rate; //!< Decay rate for neurons belonging to this vertex.
    };
    
    /*! Edge in a developmental network.
     
     Each edge corresponds to a link l \in L of D(M,L).
     */
    struct developmental_edge  {
        //! Returns true if the given graph operation is allowed.
        bool allows(graph::graph_operation::flag m) {
            return true;
        }

        //! Mutate this edge.
        template <typename EA>
        void mutate(EA& ea) {
        }

        double weight; //!< Relative weight of this edge.
    };

    //! Developmental network D(M,L).
    typedef
    boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, developmental_vertex, developmental_edge>
    developmental_network;
    
    /* Grow a graph from a template.
     */
    template <typename Graph, typename DevGraph, typename RNG>
    void delta_growth(Graph& G, std::size_t n, DevGraph& D, RNG& rng) {
        // calculate the normalized module weights in D:
        typedef std::map<typename DevGraph::vertex_descriptor, double> module_weights;
        module_weights W;
        {
            typename DevGraph::vertex_iterator vi, vi_end;
            for(boost::tie(vi,vi_end)=boost::vertices(D); vi!=vi_end; ++vi) {
                W[*vi] = D[*vi].weight;
            }
        }
        // algorithm::normalize(M.begin(), M.end(), 1.0);
        
        // track existing vertex module assignments in G:
        typedef std::set<typename Graph::vertex_descriptor> vertex_set;
        typedef std::map<typename DevGraph::vertex_descriptor, vertex_set> module_assignment;
        module_assignment M_extant, M_new;
        {
            typename Graph::vertex_iterator vi,vi_end;
            for(boost::tie(vi,vi_end)=boost::vertices(G); vi!=vi_end; ++vi) {
                M_extant[boost::vertex(G[*vi].color)].insert(*vi);
            }
        }

        // allocate n new vertices in G, and assign them to modules:
        int module=0;
        for(typename module_weights::iterator i=W.begin(); i!=W.end(); ++i, ++module) {
            // this can be replaced by a roulette wheel...
            for(std::size_t j=0; j<static_cast<std::size_t>(n*i->second); ++j) {
                typename Graph::vertex_descriptor u=boost::add_vertex(G);
                G[u].color = module;
                M_new[i->first].insert(u);
                M_extant[i->first].insert(u);
            }
        }
        
        // link up the new vertices in G:
        for(typename module_assignment::iterator i=M_new.begin(); i!=M_new.end(); ++i) {
            for(typename vertex_set::iterator j=i->second.begin(); j!=i->second.end(); ++j) {
                // source vertex:
                typename Graph::vertex_descriptor u=*j;
                
                // possible other modules are based on the links of m in D;
                typename DevGraph::vertex_descriptor m=i->first;
                typename DevGraph::out_edge_iterator ei,ei_end;
                for(boost::tie(ei,ei_end)=boost::out_edges(m,D); ei!=ei_end; ++ei) {
                    if(rng.p(D[*ei].weight)) {
                        // other module:
                        vertex_set& n=M_extant[boost::target(*ei,D)];
                        typename Graph::vertex_descriptor v=*rng.choice(n.begin(), n.end());
                        boost::add_edge(u,v,G);
                    }
                }
            }
        }
    }
}

#endif
