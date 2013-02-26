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

LIBEA_MD_DECL(ANN_INPUTS_N, "ann.inputs.n", int);
LIBEA_MD_DECL(ANN_OUTPUTS_N, "ann.outputs.n", int);
LIBEA_MD_DECL(ANN_RANDOMIZE_N, "ann.randomize.n", int);


namespace ea {
    
    /*! Generates a random artificial neural network.
     */
    struct random_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            // and randomize it some:
            ancestors::random_graph rg;
            rg.randomize(G, get<ANN_RANDOMIZE_N>(ea), ea);
            return G;
        }
    };

    /*! Generates a minimal random artificial neural network.
     */
    struct minimal_random_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            ancestors::random_graph rg;
            while(boost::num_vertices(G) < get<ANN_RANDOMIZE_N>(ea)) {
                rg.randomize(G,1,ea);
            }
            return G;
        }
    };

    /*! Type for vertices.
     */
    template <typename Neuron>
    struct neuroevolution : Neuron {
        typedef Neuron base_type;
        typedef typename base_type::synapse_type base_synapse_type;
        
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
        }
        
        //! Mutate this neuron.
        template <typename EA>
        void mutate(EA& ea) {
        }
    };
    
    
} // ea

#endif
