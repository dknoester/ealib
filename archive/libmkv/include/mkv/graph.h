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
#ifndef _MKV_GRAPH_H_
#define _MKV_GRAPH_H_

#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <string>
#include <mkv/markov_network.h>

namespace mkv {
    
    /*! Vertex properties for Markov graphs.
     */
    struct vertex_properties {
        enum node_type { NONE, INPUT, OUTPUT, HIDDEN, GATE };
        enum gate_type { LOGIC, MARKOV, ADAPTIVE };
        vertex_properties() : nt(NONE), gt(LOGIC), idx(0) {
        }
        
        node_type nt;
        gate_type gt;
        int idx;
    };

    /*! Edge properties for Markov graphs (currently unused).
     */
    struct edge_properties {
        enum edge_type { NONE, REINFORCE, INHIBIT };
        edge_properties(edge_type e=NONE) : et(e) {
        }
        edge_type et;
    };
    
    //! Markov graph type.
    typedef boost::adjacency_list<boost::vecS,
    boost::vecS,
    boost::bidirectionalS,
    vertex_properties,
    edge_properties> markov_graph;
    
    /*! Predicate that indicates edges that do not contribute to the function
     of the Markov network.
     */
    struct reduced_edge {
        reduced_edge(markov_graph& g) : _g(g) {
        }
        
        bool operator()(markov_graph::edge_descriptor e);
        
        markov_graph& _g;
    };
    
    //! Helper function that indicates when a vertex has any edges.
    template <typename VertexDescriptor, typename Graph>
    bool has_edges(VertexDescriptor u, const Graph& g) {
        return (boost::in_degree(u,g) > 0) || (boost::out_degree(u,g) > 0);
    }
    
    namespace detail {
    
        /*! Visitor, used to build the graph for a given Markov network.
         */
        class graph_building_visitor : public boost::static_visitor< > {
        public:
            //! Constructor.
            graph_building_visitor(markov_graph::vertex_descriptor vd, markov_graph& g) : v(vd), G(g) {
            }
            
            void add_edges(const index_list_type& inputs, const index_list_type& outputs) const {
                for(std::size_t i=0; i<inputs.size(); ++i) {
                    boost::add_edge(boost::vertex(inputs[i],G), v, G);
                }
                for(std::size_t i=0; i<outputs.size(); ++i) {
                    boost::add_edge(v, boost::vertex(outputs[i],G), G);
                }
            }
            
            //! Parse a logic gate.
            void operator()(logic_gate& g) const {
                G[v].gt = vertex_properties::LOGIC;
                add_edges(g.inputs, g.outputs);
            }
            
            //! Parse a Markov gate.
            void operator()(markov_gate& g) const {
                G[v].gt = vertex_properties::MARKOV;
                add_edges(g.inputs, g.outputs);
            }
            
            //! Parse an Adaptive Markov gate.
            void operator()(adaptive_gate& g) const {
                G[v].gt = vertex_properties::ADAPTIVE;
                add_edges(g.inputs, g.outputs);
                boost::add_edge(boost::vertex(g.p,G), v, edge_properties(edge_properties::REINFORCE), G);
                boost::add_edge(boost::vertex(g.n,G), v, edge_properties(edge_properties::INHIBIT), G);
            }
            
        protected:
            markov_graph::vertex_descriptor v; //!< Which vertex we're visiting.
            markov_graph& G; //!< Markov graph that we're building.
        };

    
    } // detail

    //! Returns a genetic (that is, complete) Markov graph of the given Markov network.
    markov_graph as_genetic_graph(markov_network& net);
    
    //! Returns a reduced Markov graph of the given Markov network.
    markov_graph as_reduced_graph(markov_network& net);
    
    //! Returns a causal view of the given Markov network.
    markov_graph as_causal_graph(markov_network& net);
    
    //! Outputs the given Markov graph in graphviz format.
    void write_graphviz(const std::string& title, std::ostream& out, const markov_graph& g);

} // mkv

#endif
