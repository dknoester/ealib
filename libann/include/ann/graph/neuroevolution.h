/* neural_network.h
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
#ifndef _EA_NEURAL_NETWORK_H_
#define _EA_NEURAL_NETWORK_H_

#include <ea/meta_data.h>
#include <ea/graph.h>
#include <ann/graph/neural_network.h>
#include <ann/graph/layout.h>

LIBEA_MD_DECL(ANN_INPUTS_N, "ann.inputs.n", int);
LIBEA_MD_DECL(ANN_OUTPUTS_N, "ann.outputs.n", int);
LIBEA_MD_DECL(ANN_ACTIVATION_N, "ann.activation.n", int);

namespace ealib {

    /*! Generates a random artificial neural network.
     */
    struct random_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            mutation::graph_mutator gm;
            for(int i=0; i<get<GRAPH_EVENTS_N>(ea); ++i) {
//                gm(G,ea);
            }
            return G;
        }
    };
    
    /*! Generates an MLP.
     */
    struct mlp_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            std::size_t layers[2] = {0,0};
            layers[0] = ea.rng()(std::max(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea)));
            ann::layout::mlp(G, layers, layers+1);

            return G;
        }
    };

    /*! Generates an MLP and then randomizes it slightly.
     */
    struct random_mlp_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            std::size_t layers[2] = {0,0};
            layers[0] = ea.rng()(std::max(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea)));
            ann::layout::mlp(G, layers, layers+1);
            
            mutation::graph_mutator gm;
            for(int i=0; i<get<GRAPH_EVENTS_N>(ea); ++i) {
                gm(G,ea);
            }
            return G;
        }
    };


    /*! Adaptor to add evolvability to a Neuron.
     
     We assume that evolving a neural network means that mutations operate directly
     on the neural network.  The mutation operator in ea/graph.h requires that a color
     be assigned to vertices, which is the reason for inheritance from graph::colored_vertex.
     
     If other mutation types are explored, e.g., ones that do not require a color,
     revisit this.
     */
    template <typename Neuron>
    struct neuroevolution : Neuron, graph::colored_vertex {
        typedef Neuron neuron_type;
        
        //! Synapse type for this network.
        struct synapse_type : neuron_type::synapse_type {
            typedef typename neuron_type::synapse_type base_type;
            
            //! Constructor.
            synapse_type() {
            }
            
            //! Returns true if the requested graph operation is allowed.
            bool allows(graph::graph_operation::flag m) {
                return true;
            }
            
            //! Mutate this synapse, adding weight drawn from a standard normal distribution (can be negative).
            template <typename EA>
            void mutate(EA& ea) {
                base_type::weight += ea.rng().normal_real(0, 1.0);
            }
        };
        
        //! Constructor.
        neuroevolution() {
            neuron_type::setf(ann::neuron::hidden); // neurons default to hidden.
        }
        
        //! Returns true if the requested graph operation is allowed.
        bool allows(graph::graph_operation::flag m) {
            using namespace graph;
            using namespace ann;
            switch(m) {
                case graph_operation::remove: {
                    return !neuron_type::getf(neuron::reserved);
                }
                case graph_operation::merge: {
                    return !neuron_type::getf(neuron::reserved);
                }
                case graph_operation::duplicate: {
                    return !neuron_type::getf(neuron::reserved);
                }
                case graph_operation::source: {
                    return neuron_type::getf(neuron::input) || neuron_type::getf(neuron::output) || neuron_type::getf(neuron::hidden) || neuron_type::getf(neuron::bias);
                }
                case graph_operation::target: {
                    return neuron_type::getf(neuron::hidden) || neuron_type::getf(neuron::output);
                }
                case graph_operation::mutate: {
                    return !neuron_type::getf(neuron::reserved);
                }
                default:
                    assert(false);
            }
            return false;
        }
        
        //! Mutate this neuron.
        template <typename EA>
        void mutate(EA& ea) {
        }
    };
} // ea

#endif
