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
LIBEA_MD_DECL(ANN_ANCESTOR_MUTATION_N, "ann.ancestor.mutation.n", int);


namespace ea {

    /*! Neural network mutation operator.
     
     This mutation operator focuses on building paths from input -> output.  It
     does so by first identifying source and target vertices, and then 
     */
    struct neural_network_mutation {
        template <typename Representation, typename EA>
        void operator()(Representation& G, EA& ea) {
            
            // Y: two in, one out
            
            
            // A: one in, two out
            
            
            mutation::graph_mutation gm;
            gm(G,ea);

            if(ea.rng().p(get<GRAPH_PATH_EVENT_P>(ea))) {
                mutation::detail::add_path(G,ea);
            }
        }
    };

    
    /*! Generates a random artificial neural network.
     */
    struct random_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            // and randomize it some:
            ancestors::random_graph rg;
            rg.randomize(G, get<ANN_ANCESTOR_MUTATION_N>(ea), ea);
            return G;
        }
    };
    
    
    /*! Generates a minimal random artificial neural network.
     */
    struct minsize_random_ann {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            // build the network:
            typename EA::representation_type G(get<ANN_INPUTS_N>(ea), get<ANN_OUTPUTS_N>(ea));
            
            neural_network_mutation nm;
            while(boost::num_vertices(G) < get<ANN_ANCESTOR_MUTATION_N>(ea)) {
                nm(G,ea);
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
        
        //! Returns true if the requested mutation type is allowed.
        bool allows(mutation::graph_mutation_flags::flag m) {
            using namespace mutation;
            using namespace ann;
            switch(m) {
                case graph_mutation_flags::remove: {
                    return !base_type::getf(neuron::reserved);
                }
                case graph_mutation_flags::merge: {
                    return !base_type::getf(neuron::reserved);
                }
                case graph_mutation_flags::duplicate: {
                    return !base_type::getf(neuron::reserved);
                }
                case graph_mutation_flags::source: {
                    return base_type::getf(neuron::input) || base_type::getf(neuron::output) || base_type::getf(neuron::hidden) || base_type::getf(neuron::bias);
                }
                case graph_mutation_flags::target: {
                    return base_type::getf(neuron::hidden) || base_type::getf(neuron::output);
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
