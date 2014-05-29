/* feed_forward.h
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
#ifndef _ANN_FEED_FORWARD_H_
#define _ANN_FEED_FORWARD_H_

#include <ea/ann/graph/neural_network.h>
#include <ea/ann/graph/sigmoid.h>


namespace ann {
	
	/*! Feed-forward neuron.
	 */
	template <typename Sigmoid=hyperbolic_tangent>
	struct feed_forward_neuron : public abstract_neuron {
		typedef Sigmoid sigmoid_type; //!< Sigmoid type, used for activation.

        //! Synapse type for feed forward neurons.
        struct synapse_type : public abstract_neuron::abstract_synapse {
            typedef abstract_neuron::abstract_synapse base_type;
            //! Default constructor.
            synapse_type(double w=0.05) : base_type(w) { }
        };
		
		//! Constructor.
		feed_forward_neuron() : abstract_neuron() { }
		
		/*! Feed-forward activation.
		 
		 This is fairly straightforward; the output of any given neuron is the sigmoid of
		 the weighted-sum of its inputs.
		 */
		template <typename Vertex, typename Graph, typename Filter>
		void activate(Vertex v, Graph& g, Filter& filt) {
			// for all incoming edges of this neuron, sum the link weights * source neuron outputs:
			typename Graph::in_edge_iterator ei, ei_end;
			input = 0.0;
			for(boost::tie(ei,ei_end)=boost::in_edges(v,g); ei!=ei_end; ++ei) {
				input += g[*ei].weight * g[boost::source(*ei,g)].output;
			}
			
			// the output of this neuron is the sigmoid of the input:
			output = filt(sigmoid(input));
		}
		
		sigmoid_type sigmoid; //<! Sigmoid for this neuron.
	};
	
} // ann

#endif
