/* back_propagation.h
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
#ifndef _ANN_BACK_PROPAGATION_H_
#define _ANN_BACK_PROPAGATION_H_

#include <iostream>
#include <ann/activation.h>

namespace ann {
    
	/*! Activation visitor for back propagation.
	 
	 This activation visitor records its forward path through the neural network
	 so that it can replay that path in reverse during the back propagation step.
	 */
	template <typename NeuralNetwork>
	struct back_propagation_activation : public boost::base_visitor<back_propagation_activation<NeuralNetwork> > {
		typedef boost::on_discover_vertex event_filter;
		typedef NeuralNetwork network_type;
		typedef typename network_type::vertex_descriptor vertex_descriptor;
		typedef typename network_type::neuron_map_type neuron_map_type;
		typedef typename network_type::link_map_type link_map_type;
		typedef typename network_type::activation_visitor_type activation_visitor_type;
		typedef std::deque<vertex_descriptor> visit_queue;
        
		//! Constructor.
		back_propagation_activation(network_type& g, visit_queue& visit) : _g(g), _visit(visit), _av(g) {
		}
        
		//! Activate this neuron, and record that we visited it.
		void operator()(vertex_descriptor v, const network_type& g) {
			_av(v,g);
			_visit.push_back(v);
		}
        
		//! Calculate the delta value at an output node; delta = error_i * g'(in_i).
		double delta_output(double error, vertex_descriptor v) {
			return error * _g[v].sigmoid.derivative(_g[v].input);
		}
        
		//! Calculate the delta value at a hidden node; delta = g'(input_j) * sum_i(backprop_{j,i}).
		double delta_hidden(vertex_descriptor v) {
			double sum=0.0;
			typename network_type::out_edge_iterator ei, ei_end;
			for(tie(ei,ei_end)=boost::out_edges(v,_g); ei!=ei_end; ++ei) {
				assert(_backprop.find(*ei)!=_backprop.end());
				sum += _backprop[*ei];
			}
			return sum * _g[v].sigmoid.derivative(_g[v].input);
		}
        
		//! Update the weights of all incoming edges on a node.
		void update_weights(double delta, vertex_descriptor v, double alpha) {
			typename network_type::in_edge_iterator ei, ei_end;
			for(tie(ei,ei_end)=boost::in_edges(v,_g); ei!=ei_end; ++ei) {
				_backprop[*ei] = _g[*ei].weight * delta;
				_g[*ei].weight += alpha * _g[boost::source(*ei,_g)].output * delta;
			}
		}
        
		//! Back-propagate errors for each neuron that we visited during activation.
		void back_propagate(neuron_map_type& neuron_errors, double alpha) {
			for(typename visit_queue::reverse_iterator i=_visit.rbegin(); i!=_visit.rend(); ++i) {
				vertex_descriptor v = *i;
				switch(_g[v]._type) {
					case neuron_base::OUTPUT: {
						assert(neuron_errors.find(v)!=neuron_errors.end());
						update_weights(delta_output(neuron_errors[v], v), v, alpha);
						break;
					}
					case neuron_base::HIDDEN: {
						update_weights(delta_hidden(v), v, alpha);
						break;
					}
					default: break;
				}
			}
		}
        
		network_type& _g;
		visit_queue& _visit;
		activation_visitor_type _av;
		link_map_type _backprop;
	};
    
    
	/*! Run a single back propagation iteration, and return the sum-squared error
	 prior to adjusting weights.
	 */
	template <typename NeuralNetwork, typename InputMap, typename OutputMap>
	double back_propagate(NeuralNetwork& nn, InputMap& inputs, OutputMap& expected, double alpha=0.2) {
		// first, activate the network with the backprop visitor:
		typename NeuralNetwork::neuron_map_type outputs;
		typename back_propagation_activation<NeuralNetwork>::visit_queue visit;
        
		back_propagation_activation<NeuralNetwork> av(nn, visit);
		activate(nn, av, inputs, outputs);
        
		// calculate the errors at our outputs:
		typename NeuralNetwork::neuron_map_type errors;
		double err=0.0;
		for(typename OutputMap::iterator i=outputs.begin(); i!=outputs.end(); ++i) {
			assert(expected.find(i->first)!=expected.end());
			errors[i->first] = expected[i->first] - i->second;
			err += pow(errors[i->first], 2);
		}
        
		// back-propagate the errors:
		av.back_propagate(errors, alpha);
        
		// return the error that we had prior to back-prop:
		return err;
	}
    
} // nn

#endif
