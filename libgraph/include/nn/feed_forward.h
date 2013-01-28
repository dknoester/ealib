/* feed_forward.h
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
#ifndef _NN_FEED_FORWARD_H_
#define _NN_FEED_FORWARD_H_

#include <limits>
#include <boost/graph/adjacency_list.hpp>

#include <nn/traits.h>
#include <nn/neuron.h>
#include <nn/neural_network.h>
#include <nn/sigmoid.h>


namespace nn {
	
	/*! Feed-forward neuron.
	 */
	template <typename Sigmoid>
	struct feed_forward_neuron : neuron_base {
		typedef Sigmoid sigmoid_type; //!< Sigmoid type, used for activation.
		
		//! Constructor.
		feed_forward_neuron(neuron_type t=neuron_base::INACTIVE) : neuron_base(t), input(0.0), output(0.0) { }
		
		/*! Feed-forward activation.
		 
		 This is fairly straightforward; the output of any given neuron is the sigmoid of
		 the weighted-sum of its inputs.
		 */
		template <typename Vertex, typename Graph>
		void activate(Vertex v, Graph& g) {
			// for all incoming edges of this neuron, sum the link weights * source neuron outputs:
			typename Graph::in_edge_iterator ei, ei_end;
			input = 0.0;
			for(boost::tie(ei,ei_end)=boost::in_edges(v,g); ei!=ei_end; ++ei) {
				input += g[*ei].weight * g[boost::source(*ei,g)].output;
			}
			
			// the output of this neuron is the sigmoid of the input:
			output = sigmoid(input);
		}
		
		sigmoid_type sigmoid; //<! Sigmoid for this neuron.
		double input; //!< Input to this neuron.
		double output; //!< Output from this neuron.
	};
	
	
	/*! Feed-forward link.
	 */
	struct feed_forward_link {
		//! Default constructor.
		feed_forward_link(double w=1.0) : weight(w) { }
		
		double weight; //!< Weight of this link.
	};
	
    //! Selector for feed-forward neural networks.
	struct feed_forwardS { };
	
	//! Traits type for feed-forward neural networks.
	template < >
	struct neural_network_traits<feed_forwardS> {
		typedef hyperbolic_tangent sigmoid_type;
		typedef feed_forward_neuron<sigmoid_type> neuron_type;
		typedef feed_forward_link link_type;
		typedef boost::adjacency_list<boost::setS,boost::vecS,boost::bidirectionalS,neuron_type,link_type> graph_type;
		typedef neuron_activation_visitor<graph_type> activation_visitor_type;
		double stepsize() const { return std::numeric_limits<double>::quiet_NaN(); }

		static neuron_type make_input_neuron() { return neuron_type(neuron_base::INPUT); }
		static neuron_type make_hidden_neuron() { return neuron_type(neuron_base::HIDDEN); }
		static neuron_type make_output_neuron() { return neuron_type(neuron_base::OUTPUT); }
		static neuron_type make_inactive_neuron() { return neuron_type(neuron_base::INACTIVE); }
		static link_type make_link(double weight) { return link_type(weight); }
	};
}	// nn

#endif
