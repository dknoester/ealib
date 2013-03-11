/* neural_network.h
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
#ifndef _EA_NEURAL_NETWORK_H_
#define _EA_NEURAL_NETWORK_H_

#include <ea/meta_data.h>
#include <ea/graph.h>
#include <ann/neural_network.h>
#include <ann/layout.h>

LIBEA_MD_DECL(ANN_INPUTS_N, "ann.inputs.n", int);
LIBEA_MD_DECL(ANN_OUTPUTS_N, "ann.outputs.n", int);


namespace ea {

    /*! Generates a random artificial neural network.
     */
    struct random_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            mutation::graph_mutator gm;
            for(int i=0; i<get<GRAPH_EVENTS_N>(ea); ++i) {
                gm(G,ea);
            }
            return G;
        }
    };
    

    struct mlp_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            std::size_t layers[] = {3};
            ann::layout::mlp(G, layers, layers+1);

            return G;
        }
    };


    /*! Type for vertices.
     */
    template <typename Neuron>
    struct neuroevolution : graph::mutable_vertex<Neuron> {
        typedef graph::mutable_vertex<Neuron> base_type;
        typedef graph::mutable_edge<typename base_type::synapse_type> base_synapse_type;
        
        struct synapse_type : base_synapse_type {
            //! Constructor.
            synapse_type() : base_synapse_type() {
            }
            
            //! Mutate this synapse, adding weight drawn from a standard normal distribution (can be negative).
            template <typename EA>
            void mutate(EA& ea) {
                base_synapse_type::weight += ea.rng().normal_real(0, 1.0);
            }
        };
        
        //! Constructor.
        neuroevolution() : base_type() {
            base_type::setf(ann::neuron::hidden); // neurons default to hidden.
        }
        
        //! Returns true if the requested mutation type is allowed.
        bool allows(graph::graph_operation::flag m) {
            using namespace graph;
            using namespace ann;
            switch(m) {
                case graph_operation::remove: {
                    return !base_type::getf(neuron::reserved);
                }
                case graph_operation::merge: {
                    return !base_type::getf(neuron::reserved);
                }
                case graph_operation::duplicate: {
                    return !base_type::getf(neuron::reserved);
                }
                case graph_operation::source: {
                    return base_type::getf(neuron::input) || base_type::getf(neuron::output) || base_type::getf(neuron::hidden) || base_type::getf(neuron::bias);
                }
                case graph_operation::target: {
                    return base_type::getf(neuron::hidden) || base_type::getf(neuron::output);
                }
                case graph_operation::mutate: {
                    return !base_type::getf(neuron::reserved);
                }
            }
        }
        
        //! Mutate this neuron.
        template <typename EA>
        void mutate(EA& ea) {
        }
    };
    
    
} // ea

#endif
