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
#ifndef _ANN_NEURAL_NETWORK_H_
#define _ANN_NEURAL_NETWORK_H_

#include <boost/graph/adjacency_list.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <ea/graph.h>
#include <ann/abstract_neuron.h>
#include <ann/sigmoid.h>
#include <ann/activation.h>
#include <ann/recurrent.h>
#include <ann/layout.h>

namespace ann {
	
	/*! Neural network.
     
     This class implements a generic neural network that is templated on neuron
     type.  It inherits from a boost::graph, and provides functionality that makes
     it easier to manipulate the network, as well as to activate it.
     
     \warning There are two "extra" neurons that are always added a neural network:
     the bias neuron and the top neuron.  These are always the 0'th and 1st neuron 
     in the network, respectively.
	 */
    template <typename Neuron, typename Filter=identity<double>, typename Synapse=typename Neuron::synapse_type>
    class neural_network
    : public boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, Neuron, Synapse> {
    public:
        typedef Neuron neuron_type;
        typedef Synapse synapse_type;
        typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, Neuron, Synapse> base_type;
        typedef typename base_type::vertex_descriptor vertex_descriptor;
        typedef typename base_type::edge_descriptor edge_descriptor;
        typedef Filter filter_type;
        typedef neuron_activation_visitor<neural_network,filter_type> activation_visitor_type;

        //! Default constructor.
        neural_network() : base_type(2), _nin(0), _nout(0) {
            // vertex 0 is the "bias" neuron; it is automatically connected to all
            // active neurons, and is itself inactive:
            neuron(vertex(0)).flags(neuron::reserved | neuron::bias);
            // we're not guaranteed that the bias will be visited during activation.
            // it always outputs "1":
            neuron(vertex(0)).input = -1.0;
            neuron(vertex(0)).output = -1.0;
            
            // vertex 1 is the "top" neuron; activation is a BFS from this vertex:
            neuron(vertex(1)).flags(neuron::reserved | neuron::top);
        }
        
        //! Constructor.
        neural_network(std::size_t nin, std::size_t nout, const filter_type& f=filter_type())
        : base_type(nin+nout+2), _nin(nin), _nout(nout), _filt(f) {
            // vertex 0 is the "bias" neuron; it is automatically connected to all
            // active neurons, and is itself inactive:
            neuron(vertex(0)).flags(neuron::reserved | neuron::bias);
            // we're not guaranteed that the bias will be visited during activation.
            // it always outputs "1":
            neuron(vertex(0)).input = -1.0;
            neuron(vertex(0)).output = -1.0;
            
            // vertex 1 is the "top" neuron; activation is a BFS from this vertex:
            neuron(vertex(1)).flags(neuron::reserved | neuron::top);
            
            // connect the top neuron to the inputs, mark them:
            for(std::size_t i=2; i<(nin+2); ++i) {
                add_edge(vertex(1), vertex(i));
                neuron(vertex(i)).flags(neuron::reserved | neuron::input);
            }
            
            // connect the output neurons to the bias, mark them:
            for(std::size_t i=(nin+2); i<boost::num_vertices(*this); ++i) {
                add_edge(vertex(0), vertex(i));
                neuron(vertex(i)).flags(neuron::reserved | neuron::output);
            }
        }
        
        //! Copy constructor.
        neural_network(const neural_network& that)
        : base_type(that), _nin(that._nin), _nout(that._nout), _filt(that._filt) {
        }
        
        //! Retrieve the number of inputs.
        std::size_t ninputs() { return _nin; }
        
        //! Retrieve the number of outputs.
        std::size_t noutputs() { return _nout; }

        //! Retrieve the neuron for vertex i.
        neuron_type& neuron(vertex_descriptor i) { return (*this)[i]; }
        
        //! Retrieve the i'th input to this network (convenience method).
        double& input(std::size_t i) { return neuron(vertex(i+2)).input; }

        //! Retrieve the i'th output from this network (convenience method).
        double& output(std::size_t i) { return neuron(vertex(i+_nin+2)).output; }

        //! Retreive the i'th vertex descriptor.
        vertex_descriptor vertex(std::size_t i) { return boost::vertex(i,*this); }

        //! Retreive the i'th output vertex descriptor.
        vertex_descriptor output_vertex(std::size_t i) { return boost::vertex(i+_nin+2,*this); }

        //! Retreive the i'th output vertex descriptor.
        vertex_descriptor input_vertex(std::size_t i) { return boost::vertex(i+2,*this); }

        //! Add a vertex to this neural network, mark it as hidden, and connect it to the bias neuron.
        vertex_descriptor add_vertex() {
            vertex_descriptor v = boost::add_vertex(*this);
            (*this)[v].setf(neuron::hidden);
            add_edge(vertex(0), v);
            return v;
        }

        //! Retrieve the synapse for edge i.
        synapse_type& synapse(edge_descriptor i) { return (*this)[i]; }
        
        //! Retrieve edge(u,v), if it exists.
        std::pair<edge_descriptor,bool> edge(vertex_descriptor u, vertex_descriptor v) { return boost::edge(u,v,*this); }
        
        //! Add edge(u,v), if it does not already exist.
        std::pair<edge_descriptor,bool> add_edge(vertex_descriptor u, vertex_descriptor v) { return boost::add_edge(u,v,*this); }
        
        //! Retrieve the filter for this NeuralNetwork.
        filter_type& filter() { return _filt; }
        
        //! Activate this neural network using a filtering activation visitor.
        template <typename ActivationVisitor>
        void activate(ActivationVisitor& av, unsigned int n=1) {
            vertex_descriptor v=vertex(1);
            for( ; n>0; --n) {
                ann::activate(v, *this, av);
            }
        }

        //! Activate this neural network using a filtering activation visitor.
        void activate(unsigned int n=1) {
            activation_visitor_type av(*this,_filt);
            activate(av,n);
        }
        
    private:
        filter_type _filt; //!< Filter to be applied to neuron activity levels.
        std::size_t _nin; //!< Number of inputs.
        std::size_t _nout; //!< Number of outputs.
        
		friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            std::string g=ea::graph::graph2string(*this);
            ar & boost::serialization::make_nvp("neural_network", g);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
} // ann

#endif
