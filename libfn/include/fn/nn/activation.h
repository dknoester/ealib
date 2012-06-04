#ifndef _NN_ACTIVATION_H_
#define _NN_ACTIVATION_H_

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>

#include <nn/neuron.h>

namespace nn {
	
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
			switch(_g[v]._type) {
				case neuron_base::INPUT: {
					_g[v].output = _g[v].input;
					break;
				}
				case neuron_base::HIDDEN: {
					_g[v].activate(v, _g);
					break;
				}
				case neuron_base::OUTPUT: {
					_g[v].activate(v, _g);
					break;
				}		
				default: break;
			}
		}
		
		graph_type& _g; //!< Underlying graph of the neural network.
	};
	

	/*! Activate a neural network using the given activation visitor.
	 
	 Apply the inputs, activate the network, and read the outputs.  "Extra" inputs,
	 or those values which are present in the input, but are not in the input
	 layer of the neural network, are ignored.  Likewise, outputs will only contain
	 the values output from the output layer.
	 */
	template <typename NeuralNetwork, typename ActivationVisitor, typename InputMap, typename OutputMap>
	void activate(NeuralNetwork& nn, ActivationVisitor& av, InputMap& inputs, OutputMap& outputs) {
		// apply the inputs:
		typename NeuralNetwork::adjacency_iterator vi, vi_end;
		for(boost::tie(vi,vi_end)=nn.input_layer(); vi!=vi_end; ++vi) {
			nn[*vi].input = inputs[*vi];
		}
		
		// activate:
		boost::breadth_first_search(nn, nn.preinput(), boost::visitor(boost::make_bfs_visitor(av)));

		// read the outputs:
		outputs.clear();
		typename NeuralNetwork::inv_adjacency_iterator ivi, ivi_end;
		for(boost::tie(ivi,ivi_end)=nn.output_layer(); ivi!=ivi_end; ++ivi) {
			outputs[*ivi] = nn[*ivi].output;
		}
	}

	
	/*! Activate a neural network using the default activation visitor.
	 
	 Apply the inputs, activate the network, and read the outputs.
	 */
	template <typename NeuralNetwork, typename InputMap, typename OutputMap>
	void activate(NeuralNetwork& nn, InputMap& inputs, OutputMap& outputs) {
		typename NeuralNetwork::activation_visitor_type av(nn);
		activate(nn, av, inputs, outputs);
	}
	
} //nn

#endif
