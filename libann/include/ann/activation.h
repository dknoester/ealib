/* activation.h
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
#ifndef _ANN_ACTIVATION_H_
#define _ANN_ACTIVATION_H_

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <ann/sigmoid.h>

namespace ann {
	
	/*! Neural network activation visitor.
	 */
	template <typename NeuralNetwork, typename Filter>
	struct neuron_activation_visitor : public boost::base_visitor<neuron_activation_visitor<NeuralNetwork,Filter> > {
		typedef boost::on_discover_vertex event_filter;
		typedef NeuralNetwork ann_type; //!< Underlying neural network type;
		typedef typename ann_type::vertex_descriptor vertex_descriptor; //!< Vertex descriptor type.
        typedef Filter filter_type;
		
        //! Constructor.
        neuron_activation_visitor(ann_type& g, filter_type& f) : _g(g), _filt(f) { }

		//! Called once for each vertex, in order of discovery.
		void operator()(vertex_descriptor v, const ann_type&) {
            typename ann_type::neuron_type& n=_g[v];

            if(n.getf(neuron::top) || n.getf(neuron::bias)) {
                return;
            } else if(n.getf(neuron::input)) {
                // input neurons are filtered and passed-through:
                n.output = _filt(n.input);
            } else {
                n.activate(v, _g, _filt);
            }
		}
		
		ann_type& _g; //!< Underlying graph of the neural network.
        filter_type& _filt; //!< Filter that is applied to the output of each active neuron.
	};
	

	/*! Activate a neural network using the given activation visitor.
	 
	 Apply the inputs, activate the network, and read the outputs.  "Extra" inputs,
	 or those values which are present in the input, but are not in the input
	 layer of the neural network, are ignored.  Likewise, outputs will only contain
	 the values output from the output layer.
	 */
	template <typename Vertex, typename Graph, typename ActivationVisitor>
	void activate(Vertex v, Graph& g, ActivationVisitor& av) {
		boost::breadth_first_search(g, v, boost::visitor(boost::make_bfs_visitor(av)));
	}

	
} //nn

#endif
