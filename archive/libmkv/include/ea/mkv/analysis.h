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
#ifndef _MKV_EA_MKV_ANALYSIS_H_
#define _MKV_EA_MKV_ANALYSIS_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <ea/analysis.h>
#include <ea/datafile.h>

#include <mkv/graph.h>
#include <mkv/markov_network.h>
#include <mkv/deep_markov_network.h>

namespace mkv {
    
    /*! Save the detailed graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct genetic_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "genetic_graph";}
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network& net = ealib::phenotype(ind,ea.rng(),ea);
            
            datafile df(get<ANALYSIS_OUTPUT>(ea,"genetic_graph.dot"));
            mkv::write_graphviz(ind, df, mkv::as_genetic_graph(net));
        }
    };
    
    
    /*! Save the dominant individual in graphviz format.
     */
    template <typename EA>
    struct reduced_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "reduced_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network& net = ealib::phenotype(ind,ea.rng(),ea);
            
            datafile df(get<ANALYSIS_OUTPUT>(ea,"reduced_graph.dot"));
            mkv::write_graphviz(ind, df, mkv::as_reduced_graph(net));
        }
    };
    
    
    /*! Save the causal graph of the dominant individual in graphviz format.
     */
    template <typename EA>
    struct causal_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "causal_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::markov_network& net = ealib::phenotype(ind,ea.rng(),ea);
            
            datafile df("causal_graph.dot");
            mkv::write_graphviz(ind, df, mkv::as_causal_graph(net));
        }
    };
    
    
    /*! Datafile for markov networks statistics.
     */
    template <typename EA>
    struct network_statistics : ealib::analysis::unary_function<EA> {
        static const char* name() { return "network_statistics"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            datafile df("network_statistics.dat");
            df.add_field("individual")
            .add_field("inputs")
            .add_field("outputs")
            .add_field("hidden")
            .add_field("gates");
            
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                mkv::markov_network& net = ealib::phenotype(*i,ea.rng(),ea);
                markov_graph G = as_reduced_graph(net);
                
                double gates=0.0, inputs=0.0, outputs=0.0, hidden=0.0;
                markov_graph::vertex_iterator vi,vi_end;
                for(boost::tie(vi,vi_end)=boost::vertices(G); vi!=vi_end; ++vi) {
                    if(!has_edges(*vi,G)) {
                        continue;
                    }
                    switch(G[*vi].nt) {
                        case vertex_properties::INPUT: {
                            ++inputs;
                            break;
                        }
                        case vertex_properties::OUTPUT: {
                            ++outputs;
                            break;
                        }
                        case vertex_properties::HIDDEN: {
                            ++hidden;
                            break;
                        }
                        case vertex_properties::GATE: {
                            ++gates;
                            break;
                        }
                        default: {
                            // shouldn't ever get here...
                            throw std::logic_error("markov_network.h::network_statistics: found a vertex with an invalid node_type.");
                        }
                    }
                }
                
                df.write(i->name()).write(inputs).write(outputs).write(hidden).write(gates).endl();
            }
        }
    };
    
    /*! Save the dominant individual in graphviz format.
     */
    template <typename EA>
    struct deep_reduced_graph : public ealib::analysis::unary_function<EA> {
        static const char* name() { return "deep_reduced_graph"; }
        
        virtual void operator()(EA& ea) {
            using namespace ealib;
            using namespace ealib::analysis;
            typename EA::individual_type& ind = analysis::find_dominant(ea);
            mkv::deep_markov_network& net = ealib::phenotype(ind,ea.rng(),ea);
            
            for(std::size_t j=0; j<net.size(); ++j) {
                mkv::markov_network& layer=net[j];
                datafile df("reduced_l" + boost::lexical_cast<std::string>(j) + ".dot");
                mkv::write_graphviz(ind + boost::lexical_cast<std::string>(j), df, mkv::as_reduced_graph(layer));
            }
        }
    };

} // mkv

#endif
