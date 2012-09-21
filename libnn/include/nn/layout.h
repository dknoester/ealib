/* layout.h
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
#ifndef _NN_LAYOUT_H
#define _NN_LAYOUT_H

#include <boost/random.hpp>
#include <ctime>
#include <vector>


namespace nn {
    namespace layout {
        
        /*! Generates a completely-connected neural network without self-recursive links.
         
         This is typically used with a Concurrent Time Recurrent Neural Network (CTRNN), which has
         been shown to be a universal smooth approximator.  To layout a CTRNN, merely specify the
         number of input, output, and hidden neurons.
         
         \todo double-check the absence of self-recurrent links
         */
        template <typename NeuralNetwork>
        void complete(NeuralNetwork& nn, std::size_t nin, std::size_t nout, std::size_t nhid) {
            typedef typename NeuralNetwork::vertex_descriptor vertex_descriptor;
            typedef std::vector<vertex_descriptor> layer;
            
            // add all the neurons:
            layer neurons;
            for(std::size_t i=0; i<nin; ++i) { neurons.push_back(nn.add_input_neuron());	}
            for(std::size_t i=0; i<nhid; ++i) {	neurons.push_back(nn.add_hidden_neuron());	}		
            for(std::size_t i=0; i<nout; ++i) {	neurons.push_back(nn.add_output_neuron());	}
            
            // build the topology:
            for(std::size_t i=0; i<neurons.size(); ++i) {
                for(std::size_t j=0; j<neurons.size(); ++j) {
                    if(i!=j) {
                        nn.link(neurons[i], neurons[j]);
                    }
                }
            }
        }
        
        
        /*! Generates a feed-forward neural network with the specified number of neurons at each layer
         and links with random weights.
         
         This is the canonical model of neural networks, also known as a Multi-Layer Perceptron (MLP).
         At each layer, all neurons are connected to each neuron in the subsequent layer.  Link weights
         are initialized to random values, each non-input neuron is connected to a bias, and the 
         resulting network is suitable for training via back-propagation.
         
         The number of neurons at each layer are specified by the values of the range [first, last).
         
         We're playing a graph theory trick here in order to keep track of the input and output
         layers.  Specifically, we link the first layer to the input() neuron, and the last layer
         to the output() neuron.  It's then trivial to get the input and output layers by querying
         for outgoing or incoming edges, respectively.
         */
        template <typename NeuralNetwork, typename InputIterator>
        void mlp(NeuralNetwork& nn, InputIterator first, InputIterator last) {
            typedef boost::mt19937 engine_type;
            typedef boost::uniform_real< > uniform_real_dist;
            typedef boost::variate_generator<engine_type&, uniform_real_dist > real_rng_type;
            engine_type engine(static_cast<unsigned int>(std::time(0)));
            real_rng_type rng(engine, uniform_real_dist(-0.5,0.5));
            layout_mlp(nn, first, last, rng);
        }
        
        
        /*! Generate a feed-forward neural network, using the passed-in RNG.
         */
        template <typename NeuralNetwork, typename InputIterator, typename RandomNumberGenerator>
        void mlp(NeuralNetwork& nn, InputIterator first, InputIterator last, RandomNumberGenerator rng) {
            typedef typename NeuralNetwork::vertex_descriptor vertex_descriptor;
            typedef std::vector<vertex_descriptor> layer;
            
            assert(first != last);	
            InputIterator next=first; ++next;
            layer this_layer, last_layer;
            
            // input layer
            for(std::size_t i=0; i<*first; ++i) {
                last_layer.push_back(nn.add_input_neuron());
            }
            
            // hidden layer(s)
            ++first; ++next;
            for(; next!=last; ++first, ++next) {
                for(std::size_t i=0; i<*first; ++i) {
                    this_layer.push_back(nn.add_hidden_neuron());
                }
                for(typename layer::iterator i=last_layer.begin(); i!=last_layer.end(); ++i) {
                    for(typename layer::iterator j=this_layer.begin(); j!=this_layer.end(); ++j) {
                        nn.link(*i, *j, rng());
                    }
                }
                this_layer.swap(last_layer);
                this_layer.clear();
            }
            
            // output layer
            for(std::size_t i=0; i<*first; ++i) {
                vertex_descriptor v = nn.add_output_neuron();
                for(typename layer::iterator j=last_layer.begin(); j!=last_layer.end(); ++j) {
                    nn.link(*j, v, rng());
                }		
            }
        }
        
        template <typename NeuralNetwork, typename InputIterator>
        void mlp_w(NeuralNetwork& nn, InputIterator first, InputIterator last, double w) {
            typedef typename NeuralNetwork::vertex_descriptor vertex_descriptor;
            typedef std::vector<vertex_descriptor> layer;
            
            assert(first != last);	
            InputIterator next=first; ++next;
            layer this_layer, last_layer;
            
            // input layer
            for(std::size_t i=0; i<*first; ++i) {
                last_layer.push_back(nn.add_input_neuron());
            }
            
            // hidden layer(s)
            ++first; ++next;
            for(; next!=last; ++first, ++next) {
                for(std::size_t i=0; i<*first; ++i) {
                    this_layer.push_back(nn.add_hidden_neuron());
                }
                for(typename layer::iterator i=last_layer.begin(); i!=last_layer.end(); ++i) {
                    for(typename layer::iterator j=this_layer.begin(); j!=this_layer.end(); ++j) {
                        nn.link(*i, *j, w);
                    }
                }
                this_layer.swap(last_layer);
                this_layer.clear();
            }
            
            // output layer
            for(std::size_t i=0; i<*first; ++i) {
                vertex_descriptor v = nn.add_output_neuron();
                for(typename layer::iterator j=last_layer.begin(); j!=last_layer.end(); ++j) {
                    nn.link(*j, v, w);
                }		
            }
        }
        
	} // layout
} //nn

#endif
