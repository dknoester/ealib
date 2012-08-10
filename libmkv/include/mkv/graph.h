/* markov_network.h
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
    
    struct vertex_properties {
        enum node_type { NONE, INPUT, OUTPUT, HIDDEN, GATE };
        
        vertex_properties() : nt(NONE), idx(0) {
        }
        
        node_type nt;
        int idx;
        markov_network::nodeptr_type node;
    };
    
    struct edge_properties {
    };
    
    typedef boost::adjacency_list<boost::setS,
    boost::vecS,
    boost::bidirectionalS,
    vertex_properties,
    edge_properties> mkv_graph;
    
    struct reduced_edge {
        reduced_edge(mkv_graph& g) : _g(g) {
        }
        
        bool operator()(mkv_graph::edge_descriptor e);
        
        mkv_graph& _g;
    };
    
    template <typename VertexDescriptor, typename Graph>
    bool has_edges(VertexDescriptor u, const Graph& g) {
        return (boost::in_degree(u,g) > 0) || (boost::out_degree(u,g) > 0);
    }
    
    mkv_graph as_genetic_graph(markov_network& h);
    mkv_graph as_reduced_graph(markov_network& h);
    mkv_graph as_causal_graph(markov_network& h);
    
    void write_graphviz(const std::string& title, std::ostream& out, const mkv_graph& g, bool detailed=false);
    
} // mkv

#endif
