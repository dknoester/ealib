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

namespace ann {
	
	/*! Neural network activation visitor.
	 */
	template <typename Graph>
	struct neuron_activation_visitor : public boost::base_visitor<neuron_activation_visitor<Graph> > {
		typedef boost::on_discover_vertex event_filter;
		typedef Graph graph_type; //!< Underlying graph type of the neural network.
		typedef typename graph_type::vertex_descriptor vertex_descriptor; //!< Vertex descriptor type.
		
		//! Constructor.
		neuron_activation_visitor(graph_type& g) : _g(g) { }
		
		//! Called once for each vertex, in order of discovery.
		void operator()(vertex_descriptor v, const graph_type&) {
            _g[v].activate(v, _g);
		}
		
		graph_type& _g; //!< Underlying graph of the neural network.
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

	
	/*! Activate a neural network using the default activation visitor.
	 
	 Apply the inputs, activate the network, and read the outputs.
	 */
	template <typename Vertex, typename Graph>
	void activate(unsigned int n, Vertex v, Graph& g) {
        neuron_activation_visitor<Graph> av(g);
        for( ; n>0; --n) {
            activate(v, g, av);
        }
	}
	
} //nn

#endif
