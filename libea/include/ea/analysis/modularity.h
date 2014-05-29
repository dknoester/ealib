/* modularity.h 
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

#ifndef _EA_ANALYSIS_MODULARITY_H_
#define _EA_ANALYSIS_MODULARITY_H_

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <numeric>
#include <vector>

namespace ealib {
    namespace analysis {
        
        struct modularity_edge_properties {
            modularity_edge_properties() : weight(0.0) { }
            modularity_edge_properties(double w) : weight(w) { }
            double weight;
        };
        
        struct modularity_vertex_properties {
            int color;
        };
        
        typedef boost::adjacency_list<boost::vecS, // allows parallel edges
        boost::vecS,
        boost::undirectedS,
        modularity_vertex_properties,
        modularity_edge_properties> modularity_graph;
            
        struct modularity_result {
            modularity_graph g;    
            std::vector<double> qn;
            double max_q;
            std::size_t removed;
            std::size_t num_modules;
        };        
        
        template <typename Graph>
        double summed_edge_weights(std::size_t v, Graph& g) {
            double w=0.0;
            typename Graph::out_edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::out_edges(boost::vertex(v,g),g); ei!=ei_end; ++ei) {
                w += g[*ei].weight;
            }
            return w;
        }

        
        
        
        
        
        template <typename Graph>
        modularity_result modularity(Graph g) {
            typedef typename Graph::edge_descriptor Edge;            
            typedef std::vector<Edge> EdgeList;
            
            modularity_result result;
            
            modularity_graph m(boost::num_vertices(g));
            typename Graph::edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                boost::add_edge(boost::source(*ei,g), boost::target(*ei,g), modularity_edge_properties(g[*ei].weight), m);
            }
            
            
            EdgeList el = girvan_newman_clustering(g);
            std::pair<std::size_t, double> maxQ(0,0.0);
            
            for(std::size_t e=0; e<el.size(); ++e) {
                std::vector<int> component(num_vertices(g));
                boost::connected_components(g, &component[0]);
                
                double m=0.0;
                for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                    m += g[*ei].weight;
                }
                    
                double sum=0.0;                
                for(std::size_t i=0; i<num_vertices(g); ++i) {
                    for(std::size_t j=(i+1); j<num_vertices(g); ++j) {
                        if(component[i] != component[j]) {
                            continue;
                        }
                        
                        double ki=summed_edge_weights(i,g);
                        double kj=summed_edge_weights(j,g);
                        double aij=0.0;
                        if(boost::edge(vertex(i,g), boost::vertex(j,g), g).second) {
                            aij = g[boost::edge(vertex(i,g), boost::vertex(j,g), g).first].weight;
                        }
                        
                        sum += aij - (ki*kj)/(2*m);
                    }
                }
                
                double Q = 1.0/(4.0 * m) * sum;
                
                result.qn.push_back(Q);
                
                if(Q > maxQ.second) {
                    maxQ = std::make_pair(e,Q);
                }
                
                boost::remove_edge(boost::source(el[e],g), boost::target(el[e],g), g);
            }
            
            // replay edge removals up to maxQ on m:
            for(std::size_t i=0; i<maxQ.first; ++i) {
                boost::remove_edge(boost::source(el[i],m), boost::target(el[i],m), m);
            }
            
            // recalc connected components:
            std::vector<int> component(boost::num_vertices(m));
            result.num_modules = boost::connected_components(m, &component[0]);

            // and assign colors:
            for(std::size_t i=0; i<component.size(); ++i) {
                m[boost::vertex(i,m)].color = component[i];
            }            
            
            result.max_q = maxQ.second;
            result.g = m;
            result.removed = maxQ.first;
            return result;
        }
        
        
        /*! Calculate Newman modularity (Q_N) of the given graph and module assignment.
         */
        template <typename Graph, typename ModuleMap>
        double newman_modularity(const Graph& g, const ModuleMap& module) {
            double m=0.0;
            typename Graph::edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                m += g[*ei].weight;
            }
            
            double sum=0.0;                
            for(std::size_t i=0; i<boost::num_vertices(g); ++i) {
                for(std::size_t j=(i+1); j<boost::num_vertices(g); ++j) {
                    if(module[i] != module[j]) {
                        continue;
                    }
                    
                    double ki=summed_edge_weights(i,g);
                    double kj=summed_edge_weights(j,g);
                    double aij=0.0;
                    if(boost::edge(vertex(i,g), boost::vertex(j,g), g).second) {
                        aij = g[boost::edge(vertex(i,g), boost::vertex(j,g), g).first].weight;
                    }
                    
                    sum += aij - (ki*kj)/(2*m);
                }
            }
            
            return 1.0/(4.0 * m) * sum;
        }
        
        
        /*! Calculate Q_n of the given graph.
         */
        template <typename Graph>
        double newman_modularity(const Graph& g) {
            std::vector<int> components(boost::num_vertices(g));
            boost::connected_components(g, &components[0]);
            return newman_modularity(g, components);
        }
        
        template <typename Graph>
        int num_components(const Graph& g) {
            std::vector<int> components(boost::num_vertices(g));
            return boost::connected_components(g, &components[0]);
        }
        
        
        /*! Calculate Hintze modularity of the given graph, module assignment,
         and number of modules.
         */
        template <typename Graph, typename ModuleMap>
        double hintze_modularity(const Graph& g, const ModuleMap& module, const int n) {
            double sum=0.0;                
            for(std::size_t i=0; i<boost::num_vertices(g); ++i) {
                for(std::size_t j=(i+1); j<boost::num_vertices(g); ++j) {
                    // if they're connected...
                    if(boost::edge(vertex(i,g), boost::vertex(j,g), g).second) {
                        if(module[i] == module[j]) {
                            // and in the same module, add the edge weight:
                            sum += g[boost::edge(vertex(i,g), boost::vertex(j,g), g).first].weight;
                        } else {
                            // and not in the same module, subtract the edge weight / number of modules:
                            sum -= g[boost::edge(vertex(i,g), boost::vertex(j,g), g).first].weight / (static_cast<double>(n)-1.0);
                        }
                    }
                }
            }

            return sum / graph_sum(g);
        }
        
        
        /*! Calculate Q_h of the given modularity graph.
         */
        template <typename Graph>
        double hintze_modularity(Graph& g) {
            std::vector<int> components(boost::num_vertices(g));
            int nc = boost::connected_components(g, &components[0]);
            return hintze_modularity(g, components, nc);
        }
        
    } // analysis
} // ea

#endif
