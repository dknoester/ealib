/* neurodevelopment.h
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
#ifndef _EA_ANN_NEURODEVELOPMENT_H_
#define _EA_ANN_NEURODEVELOPMENT_H_

#include <boost/graph/adjacency_list.hpp>

#include <ea/ann/neuroevolution.h>
#include <ea/graph.h>
#include <ea/metadata.h>

namespace ealib {
    LIBEA_MD_DECL(DEV_VERTICES_N, "ea.ann.development.vertices.n", int);
    LIBEA_MD_DECL(DEV_EVENTS_N, "ea.ann.development.events.n", int);
    
    namespace translators {
        
        /*! Phi translator, which produces a phenotype from a developmental 
         template.
         
         There's a bit of a polarity mismatch between the graph operations needed
         in <ea/graph.h> and the structure of the neural networks, so we go through
         a two-step translation process whereby we first build T, which has the
         right connectivity, and then translate that into an ANN.
         */
        struct phi {
            template <typename EA>
            phi(EA& ea) {
            }
            
            template <typename EA>
            typename EA::phenotype_type operator()(typename EA::genome_type& G, EA& ea) {
                // construct T, a bidirectional unweighted graph from a developmental template:
                boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, graph::mutable_vertex> T;
                graph::phi(T, get<DEV_VERTICES_N>(ea), G, ea.rng());
                
                std::size_t nin=get<ANN_INPUT_N>(ea);
                std::size_t nout=get<ANN_OUTPUT_N>(ea);
                assert((nin+nout) < boost::num_vertices(T));
                
                // build an ANN from T:
                typename EA::phenotype_type N(nin, nout, boost::num_vertices(T) - nin - nout);
                
                // ok, module 0 and 1 are inputs and outputs, respectively.
                // we need to make sure that those get put in the right place in the ANN.
                // the rest of them can go anywhere.

                for(std::size_t i=0; i<N.size(); ++i) {
                    for(std::size_t j=0; j<N.size(); ++j) {
                        if(boost::edge(boost::vertex(i,T), boost::vertex(j,T), T).second) {
                            // not sure if this is right; the inputs and outputs are likely to be
                            // totally confused if we're not assigning them to the right modules...
                            N(i,j) = ea.rng().normal_real(0.0, get<MUTATION_NORMAL_REAL_VAR>(ea));
                        }
                    }
                }
                
                return N;
            }
        };

        /*! Delta translator, which produces a phenotype from a delta graph.
         
         As with the Phi translator above, here we also go through a two-step 
         translation process whereby we first build T, which has the right
         connectivity, and then translate that into an ANN.
         */
        struct delta {
            template <typename EA>
            delta(EA& ea) {
            }
            
            template <typename EA>
            typename EA::phenotype_type operator()(typename EA::genome_type& G, EA& ea) {
                // construct T, a bidirectional unweighted graph from a developmental template:
                boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, graph::mutable_vertex> T;
                graph::delta_growth_n(T, get<DEV_EVENTS_N>(ea), G, ea.rng());
                
                std::size_t nin=get<ANN_INPUT_N>(ea);
                std::size_t nout=get<ANN_OUTPUT_N>(ea);
                assert((nin+nout) < boost::num_vertices(T));
                
                // build an ANN from T:
                typename EA::phenotype_type N(nin, nout, boost::num_vertices(T) - nin - nout);
                
                // ok, module 0 and 1 are inputs and outputs, respectively.
                // we need to make sure that those get put in the right place in the ANN.
                // the rest of them can go anywhere.
                
                for(std::size_t i=0; i<N.size(); ++i) {
                    for(std::size_t j=0; j<N.size(); ++j) {
                        if(boost::edge(boost::vertex(i,T), boost::vertex(j,T), T).second) {
                            // not sure if this is right; the inputs and outputs are likely to be
                            // totally confused if we're not assigning them to the right modules...
                            N(i,j) = ea.rng().normal_real(0.0, get<MUTATION_NORMAL_REAL_VAR>(ea));
                        }
                    }
                }
                
                return N;
            }
        };

    } // translators
} // ealib

#endif
