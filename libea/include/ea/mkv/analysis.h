/* mkv/analysis.h
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
#ifndef _EA_MKV_ANALYSIS_H_
#define _EA_MKV_ANALYSIS_H_

#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>
#include <string>
#include <set>

#include <ea/analysis/dominant.h>
#include <ea/datafile.h>
#include <mkv/graph.h>


namespace ealib {
    namespace analysis {
        using namespace mkv;
        
        /*! Predicate that indicates edges that do not contribute to the function
         of the Markov network.
         */
        template <typename Graph>
        struct reduced_edge {
            reduced_edge(Graph& g) : _g(g) {
            }
            
            bool operator()(typename Graph::edge_descriptor e) {
                // if the edge targets a node that is an input, remove.
                if(_g[boost::target(e,_g)].nt == vertex_properties::INPUT) {
                    return true;
                }
                
                // if the edge sources a node that is not an input, and that node is otherwise unconnected, remove.
                if((_g[boost::source(e,_g)].nt != vertex_properties::INPUT)
                   && (boost::in_degree(boost::source(e,_g),_g) == 0)) {
                    return true;
                }
                
                // if the edge targets a node that is not an output, and that node is otherwise unconnected, remove.
                if((_g[boost::target(e,_g)].nt != vertex_properties::OUTPUT)
                   && (boost::out_degree(boost::target(e,_g),_g) == 0)) {
                    return true;
                }
                
                return false;
            }
            Graph& _g;
        };
        
        //! Helper function that indicates when a vertex has any edges.
        template <typename VertexDescriptor, typename Graph>
        bool has_edges(VertexDescriptor u, const Graph& g) {
            return (boost::in_degree(u,g) > 0) || (boost::out_degree(u,g) > 0);
        }
        
        //! Breadth-first visitor that records which nodes were visited.
        template <typename NodeSet>
        struct node_recorder : public boost::default_bfs_visitor {
            node_recorder(NodeSet& ns) : visited(ns) {
            }
            
            template <typename Vertex, typename Graph>
            void discover_vertex(Vertex u, const Graph & g) const {
                visited.insert(u);
            }
            
            NodeSet& visited;
        };
        
        //! Returns a genetic (that is, complete) Markov graph of the given Markov network.
        template <typename MarkovNetwork>
        markov_graph as_genetic_graph(MarkovNetwork& net) {
            markov_graph g(net.nstates() + net.ngates());
            
            // color the states:
            int v=0; // vertex counter
            for(std::size_t i=0; i<net.ninputs(); ++i, ++v) {
                g[boost::vertex(v,g)].nt = vertex_properties::INPUT;
                g[boost::vertex(v,g)].idx = v;
            }
            for(std::size_t i=0; i<net.noutputs(); ++i, ++v) {
                g[boost::vertex(v,g)].nt = vertex_properties::OUTPUT;
                g[boost::vertex(v,g)].idx = v;
            }
            for(std::size_t i=0; i<net.nhidden(); ++i, ++v) {
                g[boost::vertex(v,g)].nt = vertex_properties::HIDDEN;
                g[boost::vertex(v,g)].idx = v;
            }
            
            // add the edges:
            for(std::size_t i=0; i<net.ngates(); ++i, ++v) {
                g[boost::vertex(v,g)].nt = vertex_properties::GATE;
                g[boost::vertex(v,g)].idx = v;
                net[i].as_graph(boost::vertex(v,g), g);
            }
            
            return g;
        }
        
        //! Returns a reduced Markov graph of the given Markov network.
        template <typename MarkovNetwork>
        markov_graph as_reduced_graph(MarkovNetwork& net) {
            markov_graph g = as_genetic_graph(net);
            
            std::size_t last_edges;
            do {
                last_edges = boost::num_edges(g);
                boost::remove_edge_if(reduced_edge<markov_graph>(g), g);
            } while(boost::num_edges(g) != last_edges);
            
            boost::reverse_graph<markov_graph> r(g);
            std::set<markov_graph::vertex_descriptor> ns;
            node_recorder<std::set<markov_graph::vertex_descriptor> > nr(ns);
            
            // iterate over the vertices in the original graph,
            // and if they're outputs, bfs in the reversed graph:
            markov_graph::vertex_iterator vi,vi_end;
            for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                if(g[*vi].nt == vertex_properties::OUTPUT) {
                    boost::breadth_first_search(r, *vi, boost::visitor(nr));
                }
            }
            
            // now, get rid of everyone that we *didn't visit*.
            // two stages to avoid problems with iterator invalidation.
            std::vector<markov_graph::vertex_descriptor> rmlist;
            for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                if(nr.visited.find(*vi) == nr.visited.end()) {
                    rmlist.push_back(*vi);
                }
            }
            
            // just remove the edges; it's cleaned up in printing.
            for(std::vector<markov_graph::vertex_descriptor>::iterator i=rmlist.begin(); i!=rmlist.end(); ++i) {
                boost::clear_vertex(*i,g);
            }
            
            return g;
        }
        
        //! Returns a causal view of the given Markov network.
        template <typename MarkovNetwork>
        markov_graph as_causal_graph(MarkovNetwork& net) {
            markov_graph g = as_reduced_graph(net);
            
            markov_graph::vertex_iterator vi,vi_end;
            
            for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                if((boost::in_degree(*vi,g) > 0) || (boost::out_degree(*vi,g) > 0)) {
                    if(g[*vi].nt == vertex_properties::GATE) {
                        markov_graph::out_edge_iterator oei,oei_end;
                        for(boost::tie(oei,oei_end)=boost::out_edges(*vi,g); oei!=oei_end; ++oei) {
                            
                            markov_graph::in_edge_iterator iei,iei_end;
                            for(boost::tie(iei,iei_end)=boost::in_edges(*vi,g); iei!=iei_end; ++iei) {
                                
                                boost::add_edge(boost::source(*iei,g),
                                                boost::target(*oei,g),
                                                g);
                            }
                        }
                        boost::clear_vertex(*vi,g);
                    }
                }
            }
            
            return g;
        }
        
        //! Outputs the given Markov graph in graphviz format.
        template <typename MarkovGraph>
        void write_graphviz(const std::string& title, std::ostream& out, const MarkovGraph& g) {
            
            out << "digraph {" << std::endl << "edge [ arrowsize=0.75 ];" << std::endl;
            out << "labelloc=\"t\"" << std::endl << "label=\"" << title << "\"" << std::endl;
            
            typename MarkovGraph::vertex_iterator vi,vi_end;
            
            out << "subgraph {" << std::endl << "rank=same;" << std::endl;
            for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::INPUT)) {
                    out << g[*vi].idx << " [color=green];" << std::endl;
                }
            }
            out << "}" << std::endl;
            
            out << "subgraph {" << std::endl << "rank=same;" << std::endl;
            for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::OUTPUT)) {
                    out << g[*vi].idx << " [color=red];" << std::endl;
                }
            }
            out << "}" << std::endl;
            
            out << "subgraph {" << std::endl << "rank=same;" << std::endl;
            for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::HIDDEN)) {
                    out << g[*vi].idx << " [color=blue];" << std::endl;
                }
            }
            out << "}" << std::endl;
            
            std::string nl="A";
            out << "subgraph {" << std::endl << "rank=same;" << std::endl;
            for(boost::tie(vi,vi_end)=boost::vertices(g); vi!=vi_end; ++vi) {
                if(has_edges(*vi,g) && (g[*vi].nt == vertex_properties::GATE)) {
                    out << g[*vi].idx << " ";
                    out << "[shape=box,label=\"" << nl << "::";
                    switch(g[*vi].gt) {
                        case vertex_properties::LOGIC: out << "L"; break;
                        case vertex_properties::MARKOV: out << "M"; break;
                        case vertex_properties::ADAPTIVE: out << "A"; break;
                    }
                    out << "\"];" << std::endl;
                    if(*nl.rbegin() == 'Z') {
                        std::string::reverse_iterator i=nl.rbegin();
                        for( ; i!=nl.rend(); ++i) {
                            if(*i == 'Z') {
                                *i = 'A';
                            } else {
                                ++*i;
                                break;
                            }
                        }
                        if(i == nl.rend()) {
                            nl.push_back('A');
                        }
                    } else {
                        ++*nl.rbegin();
                    }
                }
            }
            out << "}" << std::endl;
            
            markov_graph::edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                out << g[boost::source(*ei,g)].idx << "->" << g[boost::target(*ei,g)].idx;
                switch(g[*ei].et) {
                    case edge_properties::REINFORCE: {
                        out << " [color=green];" << std::endl;
                        break;
                    }
                    case edge_properties::INHIBIT: {
                        out << " [color=red];" << std::endl;
                        break;
                    }
                    default: {
                        out << ";" << std::endl;
                    }
                }
            }
            
            out << "}" << std::endl;
        }
        
        /*! Generates a graphviz format genetic view of the dominant Markov network.
         */
        LIBEA_ANALYSIS_TOOL(dominant_genetic_graph) {
            using namespace ealib;
            typename EA::iterator i=analysis::dominant(ea);
            typename EA::phenotype_type& P=ealib::phenotype(*i,ea);
            
            datafile df("mkv_dominant_genetic_graph.dot"); // dot file!
            
            std::ostringstream title;
            title << "name=" << get<IND_UNIQUE_NAME>(*i) << ", gen=" << get<IND_GENERATION>(*i) << " (genetic graph)";
            write_graphviz(title.str(), df, as_genetic_graph(P));
        }
        
        /*! Generates a graphviz format reduced view of the dominant Markov network.
         */
        LIBEA_ANALYSIS_TOOL(dominant_reduced_graph) {
            using namespace ealib;
            typename EA::iterator i=analysis::dominant(ea);
            typename EA::phenotype_type& P=ealib::phenotype(*i,ea);
            
            datafile df("mkv_dominant_reduced_graph.dot"); // dot file!
            
            std::ostringstream title;
            title << "name=" << get<IND_UNIQUE_NAME>(*i) << ", gen=" << get<IND_GENERATION>(*i) << " (reduced graph)";
            write_graphviz(title.str(), df, as_reduced_graph(P));
        }
        
        /*! Generates a graphviz format reduced view of a dominant Markov network for each objective.
         */
        LIBEA_ANALYSIS_TOOL(multi_reduced_graph) {
            using namespace ealib;
            
            for(std::size_t i=0; i<ea.fitness_function().size(); ++i) {
                std::sort(ea.population().begin(), ea.population().end(), comparators::objective<EA>(i,ea));
                typename EA::reverse_iterator ind=ea.rbegin();
                
                // take the "best" individual for this objective:
                typename EA::phenotype_type& P=ealib::phenotype(*ind,ea);
                
                std::ostringstream fname;
                fname << "mkv_reduced_graph_obj" << i << ".dot";
                datafile df(fname.str());
                
                std::ostringstream title;
                title << "name=" << get<IND_UNIQUE_NAME>(*ind) << ", gen=" << get<IND_GENERATION>(*ind) << " (reduced graph)";
                write_graphviz(title.str(), df, as_reduced_graph(P));
            }
        }
        
        /*! Generates a graphviz format causal view of the dominant Markov network.
         */
        LIBEA_ANALYSIS_TOOL(dominant_causal_graph) {
            using namespace ealib;
            typename EA::iterator i=analysis::dominant(ea);
            typename EA::phenotype_type& P=ealib::phenotype(*i,ea);
            
            datafile df("mkv_dominant_causal_graph.dot"); // dot file!
            
            std::ostringstream title;
            title << "name=" << get<IND_UNIQUE_NAME>(*i) << ", gen=" << get<IND_GENERATION>(*i) << " (causal graph)";
            write_graphviz(title.str(), df, as_causal_graph(P));
        }
        
    } // analysis
} // ealib

#endif
