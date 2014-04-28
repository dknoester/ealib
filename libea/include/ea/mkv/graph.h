/* graph.h
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
#ifndef _EA_MKV_GRAPH_H_
#define _EA_MKV_GRAPH_H_

#include <boost/graph/adjacency_list.hpp>

namespace ealib {
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
        
    } // mkv
} // ealib

#endif
