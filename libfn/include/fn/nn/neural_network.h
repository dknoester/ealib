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
#ifndef _NN_NEURAL_NETWORK_H_
#define _NN_NEURAL_NETWORK_H_

#include <utility>
#include <map>

#include <nn/activation.h>


namespace nn {
	
	/*! Default traits for neural networks; must be specialized by the activation type.
	 */
	template <typename ActivationType>
	struct neural_network_traits {
	};
	
	
	/*! Neural network.
	 
	 A neural network is fundamentally a graph with special properties on the nodes and links.
	 The model we're using here is that of a graph of neurons (vertices) and links (edges), G=(V,E).

	 We define three special neurons -- a pre-input neuron that is connected to all neurons in the
	 input layer; a post-output neuron that is connected to all neurons in the output layer; and a
	 bias neuron that is connected to all hidden and output-layer neurons.
	 */
	template <typename ActivationType, typename Traits=neural_network_traits<ActivationType> >
	class neural_network : public Traits::graph_type {
public:
	typedef neural_network<ActivationType, Traits> this_type; //!< This class's type.
	typedef ActivationType activation_type; //!< This neural network's activation type.
	typedef neuron_activation_visitor<this_type> activation_visitor_type; //!< Activation visitor.
	typedef Traits nn_traits_type; //!< This neural network's traits type.
	typedef typename nn_traits_type::sigmoid_type sigmoid_type; //!< Sigmoid type.
	typedef typename nn_traits_type::neuron_type neuron_type; //!< Neuron type.
	typedef typename nn_traits_type::link_type link_type; //!< Link type.
	typedef typename nn_traits_type::graph_type graph_type; //!< Underlying graph.
	typedef typename graph_type::vertex_descriptor vertex_descriptor; //!< Vertex descriptor type.
	typedef typename graph_type::edge_descriptor edge_descriptor; //!< Edge descriptor type.
	typedef typename graph_type::adjacency_iterator adjacency_iterator; //!< Adjacency iterator type.
	typedef typename graph_type::inv_adjacency_iterator inv_adjacency_iterator; //!< Adjacency iterator type.
	typedef std::map<vertex_descriptor, double> neuron_map_type; //!< Map for input to / outputs from this neural network.
	typedef std::map<edge_descriptor, double> link_map_type; //!< Map for links in this neural network.
	
	//! Default constructor.
	neural_network() {
		_input = boost::add_vertex(nn_traits_type::make_inactive_neuron(), *this);
		_output = boost::add_vertex(nn_traits_type::make_inactive_neuron(), *this);
		_bias = boost::add_vertex(nn_traits_type::make_inactive_neuron(), *this);
		(*this)[_bias].output = 1.0;
	}

	//! Initializing constructor.
	neural_network(const nn_traits_type& t) : _traits(t) {
		_input = boost::add_vertex(nn_traits_type::make_inactive_neuron(), *this);
		_output = boost::add_vertex(nn_traits_type::make_inactive_neuron(), *this);
		_bias = boost::add_vertex(nn_traits_type::make_inactive_neuron(), *this);
		(*this)[_bias].output = 1.0;
	}
	
	//! Add an input neuron.
	vertex_descriptor add_input_neuron() {
		vertex_descriptor v = boost::add_vertex(nn_traits_type::make_input_neuron(), *this);
		boost::add_edge(_input, v, *this);
		return v;
	}
	
	//! Add a hidden neuron.
	vertex_descriptor add_hidden_neuron() {
		vertex_descriptor v = boost::add_vertex(nn_traits_type::make_hidden_neuron(), *this);
		boost::add_edge(_bias, v, *this);
		return v;	
	}
	
	//! Add an output neuron.
	vertex_descriptor add_output_neuron() {
		vertex_descriptor v = boost::add_vertex(nn_traits_type::make_output_neuron(), *this);
		boost::add_edge(v, _output, *this);
		boost::add_edge(_bias, v, *this);
		return v;		
	}
	
	//! Link two neurons, i->j.
	std::pair<edge_descriptor, bool> link(vertex_descriptor i, vertex_descriptor j, double weight=0.5) {
		return boost::add_edge(i, j, nn_traits_type::make_link(weight), *this);
	}
	
	//! Retrieve the input layer neurons.
	std::pair<adjacency_iterator, adjacency_iterator> input_layer() {
		return boost::adjacent_vertices(_input, *this);
	}
	
	//! Retrieve the n'th input vertex descriptor.
	vertex_descriptor input(std::size_t n) {
		adjacency_iterator i = input_layer().first;
		std::advance(i,n);
		return *i;
	}
	
	//! Retrieve the output layer neurons.
	std::pair<inv_adjacency_iterator, inv_adjacency_iterator> output_layer() {
		return boost::inv_adjacent_vertices(_output, *this);
	}
	
	//! Retrieve the n'th output vertex descriptor.
	vertex_descriptor output(std::size_t n) {
		inv_adjacency_iterator i = output_layer().first;
		std::advance(i,n);
		return *i;
	}	
	
	//! Retrieve the pre-input neuron's vertex descriptor.
	vertex_descriptor preinput() { return _input; }
	
	//! Retrieve the post-output neuron's vertex descriptor.
	vertex_descriptor postoutput() { return _output; }
	
	//! Retrieve the bias neuron's vertex descriptor.
	vertex_descriptor bias() { return _bias; }
	
	//! Retrieve this neural network's step size from its traits object.
	double stepsize() const { return _traits.stepsize(); }
	
protected:
	vertex_descriptor _input; //!< Connects to each neuron in the input layer.
	vertex_descriptor _output; //!< Connects to each neuron in the output layer.
	vertex_descriptor _bias; //!< Bias applied to all non-input neurons.
	nn_traits_type _traits; //!< This neural network's traits.
};

} // nn

#endif
