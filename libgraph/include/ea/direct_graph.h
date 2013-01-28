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
#ifndef _GRAPH_EA_GRAPH_H_
#define _GRAPH_EA_GRAPH_H_

LIBEA_MD_DECL(GRAPH_RESERVED_VERTICES, "graph.reserved_vertices", int);
LIBEA_MD_DECL(GRAPH_ADD_EDGE_P, "graph.min_vertices", double);
LIBEA_MD_DECL(GRAPH_REMOVE_EDGE_P, "graph.min_vertices", double);
LIBEA_MD_DECL(GRAPH_ALTER_EDGE_P, "graph.min_vertices", double);
LIBEA_MD_DECL(GRAPH_INITIAL_EDGES, "graph.initial_edges", int);


namespace ea {
    namespace mutation {
        
        /*! Direct graph mutation operator.
         
         Each possible mutation type (vertex & edge deletions, insertions, changes)
         must be defined for a direct graph representation.
         */
        struct direct_graph_mutation {
            template <typename Representation, typename EA>
            void operator()(Representation& repr, EA& ea) {
                using namespace boost;
                // Representation is assumed to support a boost::adjacency_list compatible interface,
                // specifically: boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS...>
                
                // add edge (may create a new vertex):
                if(get<GRAPH_ADD_EDGE_P>(ea)) {
                    Representation::vertex_descriptor u,v;
                    tie(u,v) = ea.rng().choose_two(0, num_vertices(repr)+1);
                    if(v == num_vertices(repr)) {
                        v = add_vertex(repr);
                        add_edge(v, vertex(ea.rng()(num_vertices(repr)),repr));
                    }
                 
                    add_edge(u, v, repr);
                }

                // remove edge (may remove a vertex):
                if(ea.rng().p(get<GRAPH_REMOVE_EDGE_P>(ea))) {
                    Representation::edge_descriptor e=edge(ea.rng()(num_edges(repr)), repr);
                    Representation::vertex_descriptor u=source(e,repr);
                    Representation::vertex_descriptor v=target(e,repr);
                    
                    remove_edge(e, repr);
                    
                    if((u > get<GRAPH_RESERVED_VERTICES>(ea))
                       && ((in_degree(u,repr) + out_degree(u,repr)) == 0)) {
                        remove_vertex(u,repr);
                    }
                    
                    if((v > get<GRAPH_RESERVED_VERTICES>(ea))
                       && ((in_degree(v,repr) + out_degree(v,repr)) == 0)) {
                        remove_vertex(v,repr);
                    }
                }
                
                // alter an edge:
                if(ea.rng().p(get<GRAPH_ALTER_EDGE_P>(ea))) {
                    Representation::edge_descriptor e=edge(ea.rng()(num_edges(repr)), repr);
                    repr[e].mutate(ea); // or something like this...
                }
            }
        };


        /*! Generates a random graph-based individual.
         */
        struct random_graph_individual {
            template <typename EA>
            typename EA::representation_type operator()(EA& ea) {
                typename EA::representation_type repr(get<GRAPH_RESERVED_VERTICES>(ea));

                for(int i=get<GRAPH_INITIAL_EDGES>(ea); i>0; --i) {
                    Representation::vertex_descriptor u,v;
                    tie(u,v) = ea.rng().choose_two(0, num_vertices(repr)+1);
                    if(v == num_vertices(repr)) {
                        v = add_vertex(repr);
                        add_edge(v, vertex(ea.rng()(num_vertices(repr)),repr));
                    }
                
                    add_edge(u, v, repr);
                }

                return repr;
            }
        };
    
    // helper methods
    
    // datafiles, events
    
} // ea

#endif
