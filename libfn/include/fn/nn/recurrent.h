/* recurrent.h
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
#ifndef _NN_RECURRENT_H_
#define _NN_RECURRENT_H_

#include <boost/graph/adjacency_list.hpp>

#include <nn/neural_network.h>
#include <nn/sigmoid.h>


namespace nn {
	
	/*! Recurrent neuron.
	 */
	template <typename Sigmoid>
	struct recurrent_neuron : public neuron_base {
		typedef Sigmoid sigmoid_type; //!< Sigmoid type, used for activation.
		
		//! Constructor.
		recurrent_neuron(neuron_type t=neuron_base::INACTIVE) : neuron_base(t), input(0.0), output(0.0) { }
		
		/*! Recurrent activation.
		 
		 Recurrent neural networks (RNNs) are a slightly different beast than feed-forward 
		 neural networks in that activations proceed according to the following function:
		 y_i(t) = f_i(net_i(t-1)),
		 that is, the activation (output) at time t of any given neuron in the network depends 
		 on its inputs from time t-1.  Thus, activations proceed along edges, and asymmetries in
		 the network serve as delays.
		 
		 The activation of a neuron then proceeds as follows:
		 - apply inputs
		 - for each neuron n:
		 activation = 0
		 for each incoming edge e:
		 activation += w(e) * e(t-1)
		 e(t-1) = e(t)
		 activation = sigmoid(activation)
		 for each outgoing edge e:
		 e(t) = activation
		 
		 The end result here is that we have different methods of evaluation for different neural
		 network structures.  This is fine; just have to be careful when we start comparing between
		 them.  With feed-forward networks, each evaluation is a complete input->output strobe,
		 while with recurrent networks it's an update on each neuron.
		 
		 A consequence of RNN activation is that the time slice for each update must be smaller
		 that that for feed-forward networks in order for the outputs to correlate to the inputs
		 in a reasonable amount of time.  Since evaluation of a feed-forward network is complete
		 (input signals propagate entirely to outputs), in RNNs input signals only propagate a
		 single layer. 
		 */
		template <typename Vertex, typename Graph>
		void activate(Vertex v, Graph& g) {
			// for all incoming edges of this neuron, sum the link weights * link value(t-1):
			typename Graph::in_edge_iterator ei, ei_end;
			input = 0.0;
			for(boost::tie(ei,ei_end)=boost::in_edges(v,g); ei!=ei_end; ++ei) {
				input += g[*ei].weight * g[*ei].t_minus1;
				g[*ei].t_minus1 = g[*ei].t; // rotate present->past once it's been used.
			}
			
			// the output of this vertex is the sigmoid of all the inputs:
			output = sigmoid(input);
			
			// update the outgoing edges:
			typename Graph::out_edge_iterator oi, oi_end;
			for(boost::tie(oi,oi_end)=boost::out_edges(v,g); oi!=oi_end; ++oi) {
				g[*oi].t = output;
			}
		}
		
		sigmoid_type sigmoid; //<! Sigmoid for this neuron.
		double input; //!< Input to this neuron.
		double output; //!< Output from this neuron.
	};
	
	
	/*! Recurrent link.
	 */
	struct recurrent_link {
		//! Default constructor.
		recurrent_link(double w=1.0) : weight(w), t(0.0), t_minus1(0.0) { }
		
		double weight; //!< Weight of this link.
		double t; //!< Value of this link at time t (present).
		double t_minus1; //!< Value of this link at time t-1 (past).
	};
	
	
	//! Selector for recurrent neural networks.
	struct recurrent_s { };
	
	
	//! Traits type for recurrent neural networks.
	template < >
	struct neural_network_traits<recurrent_s> {
		typedef hyperbolic_tangent sigmoid_type;	
		typedef recurrent_neuron<sigmoid_type> neuron_type;
		typedef recurrent_link link_type;
		typedef boost::adjacency_list<boost::setS,boost::vecS,boost::bidirectionalS,neuron_type,link_type> graph_type;
		
		static neuron_type make_input_neuron() { return neuron_type(neuron_base::INPUT); }
		static neuron_type make_hidden_neuron() { return neuron_type(neuron_base::HIDDEN); }
		static neuron_type make_output_neuron() { return neuron_type(neuron_base::OUTPUT); }
		static neuron_type make_inactive_neuron() { return neuron_type(neuron_base::INACTIVE); }
		static link_type make_link(double weight) { return link_type(weight); }
	};
	
	
	//! Convenience typedef for recurrent neural networks.
	typedef neural_network<recurrent_S> rnn;
	
} // nn

#endif
