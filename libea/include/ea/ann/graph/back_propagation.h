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

#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>
#include <ea/ann/graph/activation.h>

namespace ann {
    
	/*! Activation visitor for back propagation.
	 
	 This activation visitor records its forward path through the neural network
	 so that it can replay that path in reverse during the back propagation step.
	 */
    template <typename NeuralNetwork, typename Filter=typename NeuralNetwork::filter_type>
	struct back_propagation_activation : public boost::base_visitor<back_propagation_activation<NeuralNetwork,Filter> > {
		typedef boost::on_discover_vertex event_filter;
		typedef NeuralNetwork ann_type;
		typedef typename ann_type::vertex_descriptor vertex_descriptor;
        typedef typename ann_type::edge_descriptor edge_descriptor;
        typedef std::map<edge_descriptor,double> edge_map_type;
        typedef std::map<vertex_descriptor,double> error_map_type;
        typedef typename ann_type::activation_visitor_type activation_visitor_type;
		typedef std::deque<vertex_descriptor> visit_queue;
        typedef Filter filter_type;
        
		//! Constructor.
		back_propagation_activation(ann_type& g, visit_queue& visit, filter_type& f) : _g(g), _visit(visit), _av(g,f) {
		}
        
		//! Activate this neuron, and record that we visited it.
		void operator()(vertex_descriptor v, const ann_type& g) {
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
			typename ann_type::out_edge_iterator ei, ei_end;
			for(tie(ei,ei_end)=boost::out_edges(v,_g); ei!=ei_end; ++ei) {
                // there is the possibility that hidden neurons are unconnected to outputs, in which
                // case they contribute nothing to error.
                typename edge_map_type::iterator i=_backprop.find(*ei);
                if(i != _backprop.end()) {
                    sum += i->second;
                }
			}
			return sum * _g[v].sigmoid.derivative(_g[v].input);
		}
        
		//! Update the weights of all incoming edges on a node.
		void update_weights(double delta, vertex_descriptor v, double alpha) {
			typename ann_type::in_edge_iterator ei, ei_end;
			for(tie(ei,ei_end)=boost::in_edges(v,_g); ei!=ei_end; ++ei) {
                double bp = _g[*ei].weight * delta;
				_backprop[*ei] = bp;
                double wd = alpha * _g[boost::source(*ei,_g)].output * delta;
				_g[*ei].weight += wd;
			}
		}
        
		//! Back-propagate errors for each neuron that we visited during activation.
		void back_propagate(error_map_type& neuron_errors, double alpha) {
			for(typename visit_queue::reverse_iterator i=_visit.rbegin(); i!=_visit.rend(); ++i) {
				vertex_descriptor v = *i;
                
                if(_g[v].getf(neuron::output)) {
                    assert(neuron_errors.find(v)!=neuron_errors.end());
                    update_weights(delta_output(neuron_errors[v], v), v, alpha);
                } else if(_g[v].getf(neuron::hidden)) {
                    update_weights(delta_hidden(v), v, alpha);
                }
			}
		}
        
		ann_type& _g;
		visit_queue& _visit;
		activation_visitor_type _av;
		edge_map_type _backprop;
	};
    
    
	/*! Run n back propagation iterations against all data in input, and return
     the sum-squared error prior to adjusting weights.
	 */
	template <typename NeuralNetwork, typename Matrix>
	double back_propagate(NeuralNetwork& N, Matrix& input, Matrix& expected, double n=1, double alpha=0.05) {
        typedef back_propagation_activation<NeuralNetwork> bp_type;
        double err=0.0;

        for(; n>0; --n) {
            err = 0.0;
            for(std::size_t i=0; i<input.size1(); ++i) {
                for(std::size_t j=0; j<input.size2(); ++j) {
                    N.input(j) = input(i,j);
                }
            
                typename bp_type::visit_queue visit;
                back_propagation_activation<NeuralNetwork> av(N, visit, N.filter());
                N.activate(av,1);
            
                typename bp_type::error_map_type errors;
                for(std::size_t j=0; j<expected.size2(); ++j) {
                    errors[N.output_vertex(j)] = expected(i,j) - N.output(j);
                    err += pow(errors[N.output_vertex(j)], 2.0);
                }
                
                // back-propagate the errors:
                av.back_propagate(errors, alpha);
            }
        }
        
		// return the error that we had prior to back-prop:
		return err;
	}
    
} // ann

#endif
