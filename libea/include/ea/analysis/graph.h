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

#ifndef _EA_ANALYSIS_GRAPH_H_
#define _EA_ANALYSIS_GRAPH_H_

#include <ostream>
#include <boost/graph/connected_components.hpp>

namespace ea {
    namespace analysis {
        
        /*! Calculates the difference d of graphs g and h: d = g - h.
         
         Graph d contains all edges in g that are not in h.  The orders of g and 
         h must be the same.
         */
        template <typename Graph>
        Graph graph_difference(const Graph& g, const Graph& h) {
            assert(boost::num_vertices(g) == boost::num_vertices(h));
            
            Graph d(boost::num_vertices(g));
            typedef typename Graph::edge_iterator edge_iterator;
            edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                if(!boost::edge(boost::source(*ei,g), boost::target(*ei,g), h).second) {
                    boost::add_edge(boost::source(*ei,g), boost::target(*ei,g), g[*ei], d);
                }
            }
            
            return d;
        }
        
        
        /*! Calculates the symmetric difference d of graphs g and h: d = (g-h) + (h-g).
         
         Graph d contains all edges that in either g or h but not both.  The orders
         of g and h must be the same.
         */
        template <typename Graph>
        Graph graph_symmetric_difference(const Graph& g, const Graph& h) {
            assert(boost::num_vertices(g) == boost::num_vertices(h));
            Graph d1=graph_difference(g,h);
            Graph d2=graph_difference(h,g);
            
            typedef typename Graph::edge_iterator edge_iterator;
            edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(d2); ei!=ei_end; ++ei) {
                if(!boost::edge(boost::source(*ei,d2), boost::target(*ei,d2), d1).second) {
                    boost::add_edge(boost::source(*ei,d2), boost::target(*ei,d2), d2[*ei], d1);
                }
            }
            
            return d1;
        }
        
        
        /*! Calculates the weight difference among edges in g and h.
         
         
         */
        template <typename Graph>
        Graph graph_weight_difference(const Graph& g, const Graph& h, double threshold) {
            assert(boost::num_vertices(g) == boost::num_vertices(h));
            
            Graph d(boost::num_vertices(g));
            typedef typename Graph::edge_descriptor edge_descriptor;
            typedef typename Graph::edge_iterator edge_iterator;
            edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                edge_descriptor e;
                bool has_edge;
                tie(e,has_edge) = boost::edge(boost::source(*ei,g), boost::target(*ei,g), h);
                
                if(has_edge) {
                    double gw = g[*ei].weight;
                    double hw = h[e].weight;
                    
                    if(fabs(gw-hw) > threshold) {
                        edge_descriptor de = boost::add_edge(boost::source(*ei,g), boost::target(*ei,g), g[*ei], d).first;
                        d[de].weight -= h[e].weight;
                    }
                } else {
                    boost::add_edge(boost::source(*ei,g), boost::target(*ei,g), g[*ei], d);
                }
            }
            return d;
        }        
        
        
        /*! Normalize the edge weights in graph g to [0..1].
         */
        template <typename Graph>
        void graph_normalize(Graph& g) {
            typedef typename Graph::edge_iterator edge_iterator;
            
            double max_w=0.0;
            edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                double e = g[*ei].weight;
                if(fabs(e) > max_w) {
                    max_w = fabs(e);
                }
                
            }
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                g[*ei].weight = g[*ei].weight / max_w;
            }
        }
        
        template <typename Graph>
        void graph_abs(Graph& g) {
            typedef typename Graph::edge_iterator edge_iterator;
            
            edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                g[*ei].weight = fabs(g[*ei].weight);
            }
        }
        
        template <typename Graph>
        void graph_scale(double s, Graph& g) {
            typedef typename Graph::edge_iterator edge_iterator;
            
            edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                g[*ei].weight *= s;
            }
        }
        
        template <typename Graph>
        double graph_sum(Graph& g) {
            typedef typename Graph::edge_iterator edge_iterator;
            double s=0.0;
            edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                s += g[*ei].weight;
            }
            return s;
        }
        
        template <typename Graph>
        int color_components(Graph& g) {
            
            // recalc connected components:
            std::vector<int> component(boost::num_vertices(g));
            int nc = boost::connected_components(g, &component[0]);
            
            // and assign colors:
            for(std::size_t i=0; i<component.size(); ++i) {
                g[boost::vertex(i,g)].color = component[i];
            }            
            return nc;
        }
        
        
        /*! Write a colored circular graph to the specified output stream.
         */
        template <typename Graph>
        void write_colored_circular_graph(Graph& g, std::ostream& out) {
            out << "graph G {" << std::endl << "layout=neato;" << std::endl
            << "splines=true;" << std::endl;
            
            for(std::size_t i=0; i<boost::num_vertices(g); ++i) {
                double deg = i * (360.0/static_cast<double>(boost::num_vertices(g)));
                double rad = deg * M_PI/180.0;
                double x = 1000.0 * cos(rad);
                double y = 1000.0 * sin(rad);
                out << i << " [pos=\"" << x << "," << y << "!\",label=\"" << i << "-" << g[boost::vertex(i,g)].annotation << "\",color=";

                switch(g[boost::vertex(i,g)].color % 8) {
                    case 0: out << "black"; break;
                    case 1: out << "blue"; break;
                    case 2: out << "green"; break;
                    case 3: out << "red"; break;
                    case 4: out << "yellow"; break;
                    case 5: out << "orange"; break;
                    case 6: out << "lightblue"; break;
                    case 7: out << "khaki"; break;
                }
                if(g[boost::vertex(i,g)].bgcolor != 0) {
                    out << ",style=filled,fillcolor=";
                    switch(g[boost::vertex(i,g)].bgcolor % 8) {
                        case 0: out << "black"; break;
                        case 1: out << "blue"; break;
                        case 2: out << "green"; break;
                        case 3: out << "red"; break;
                        case 4: out << "yellow"; break;
                        case 5: out << "orange"; break;
                        case 6: out << "lightblue"; break;
                        case 7: out << "khaki"; break;
                    }
                    
                }
                out << "];" << std::endl;
            }
            
            typename Graph::edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                int lw = ceil(fabs(g[*ei].weight) / 0.1);
                out << boost::source(*ei,g) << " -- " << boost::target(*ei,g);
                out << " [style=\"setlinewidth(" << lw << ")\",";
                double deg = boost::source(*ei,g) * (360.0/100.0);
                double rad = deg * M_PI/180.0;
                double x = cos(rad);
                double y = sin(rad);
                double close=975.0,far=800.0;
                out << "pos=\"" << close*x << "," << close*y << " " << far*x << "," << far*y;
                deg = boost::target(*ei,g) * (360.0/100.0);
                rad = deg * M_PI/180.0;
                x = cos(rad);
                y = sin(rad);                                                
                out << " " << far*x << "," << far*y << " " << close*x << "," << close*y << "\"];" << std::endl;
            }                
            out << "}" << std::endl;
        }
        
        template <typename Graph>
        void write_circular_graph(Graph& g, std::ostream& out) {
            out << "graph G {" << std::endl << "layout=neato;" << std::endl
            << "splines=true;" << std::endl;
            
            for(std::size_t i=0; i<boost::num_vertices(g); ++i) {
                double deg = i * (360.0/static_cast<double>(boost::num_vertices(g)));
                double rad = deg * M_PI/180.0;
                double x = 1000.0 * cos(rad);
                double y = 1000.0 * sin(rad);
                out << i << " [pos=\"" << x << "," << y << "!\",label=\"" << i << "-" << g[boost::vertex(i,g)].annotation << "\"];" << std::endl;
            }
            
            typename Graph::edge_iterator ei,ei_end;
            for(boost::tie(ei,ei_end)=boost::edges(g); ei!=ei_end; ++ei) {
                int lw = ceil(fabs(g[*ei].weight) / 0.1);
                out << boost::source(*ei,g) << " -- " << boost::target(*ei,g);
                out << " [style=\"setlinewidth(" << lw << ")\",";
                double deg = boost::source(*ei,g) * (360.0/100.0);
                double rad = deg * M_PI/180.0;
                double x = cos(rad);
                double y = sin(rad);
                double close=975.0,far=800.0;
                out << "pos=\"" << close*x << "," << close*y << " " << far*x << "," << far*y;
                deg = boost::target(*ei,g) * (360.0/100.0);
                rad = deg * M_PI/180.0;
                x = cos(rad);
                y = sin(rad);                                                
                out << " " << far*x << "," << far*y << " " << close*x << "," << close*y << "\"];" << std::endl;
            }                
            out << "}" << std::endl;
        }
        
    } // analysis
} // ea

#endif
