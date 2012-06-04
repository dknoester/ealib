#ifndef _NN_CTRNN_H_
#define _NN_CTRNN_H_

#include <boost/graph/adjacency_list.hpp>

#include <nn/neural_network.h>
#include <nn/sigmoid.h>


namespace nn {
	
	/*! CTRNN neuron.
	 */
	template <typename Sigmoid>
	struct ctrnn_neuron : public neuron_base {
		typedef Sigmoid sigmoid_type; //!< Sigmoid type, used for activation.
		
		//! Constructor.
		ctrnn_neuron(neuron_type t=neuron_base::INACTIVE) 
		: neuron_base(t), input(0.0), output(0.0), state(0.0), tau(1.0), gain(1.0) {
		}
				
		/*! Activation function for a CTRNN neuron.
		 
		 Activation of a CTRNN is based on the following equation:
		 
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
			
			// update the state of this neuron:
			state += g.stepsize() * tau * (input - state);
			
			// update the outgoing edges with this neuron's output:
			output = sigmoid(gain * state);

			typename Graph::out_edge_iterator oi, oi_end;
			for(boost::tie(oi,oi_end)=boost::out_edges(v,g); oi!=oi_end; ++oi) {
				g[*oi].t = output;
			}
			
			/*
			 // Integrate a circuit one step using 4th-order Runge-Kutta.

			 void CTRNN::RK4Step(double stepsize)
			 {
			 int i,j;
			 double input;
			 
			 // The first step.
			 for (i = 1; i <= size; i++) {
			 input = externalinputs[i];
			 for (j = 1; j <= size; j++)
			 input += weights[j][i] * outputs[j];
			 k1[i] = stepsize * Rtaus[i] * (input - states[i]); 
			 TempStates[i] = states[i] + 0.5*k1[i];
			 TempOutputs[i] = sigmoid(gains[i]*(TempStates[i]+biases[i]));
			 }
			 
			 // The second step.
			 for (i = 1; i <= size; i++) {
			 input = externalinputs[i];
			 for (j = 1; j <= size; j++)
			 input += weights[j][i] * TempOutputs[j];
			 k2[i] = stepsize * Rtaus[i] * (input - TempStates[i]);
			 TempStates[i] = states[i] + 0.5*k2[i];
			 }
			 for (i = 1; i <= size; i++)
			 TempOutputs[i] = sigmoid(gains[i]*(TempStates[i]+biases[i]));
			 
			 // The third step.
			 for (i = 1; i <= size; i++) {
			 input = externalinputs[i];
			 for (j = 1; j <= size; j++)
			 input += weights[j][i] * TempOutputs[j];
			 k3[i] = stepsize * Rtaus[i] * (input - TempStates[i]);
			 TempStates[i] = states[i] + k3[i];
			 }
			 for (i = 1; i <= size; i++)
			 TempOutputs[i] = sigmoid(gains[i]*(TempStates[i]+biases[i]));
			 
			 // The fourth step.
			 for (i = 1; i <= size; i++) {
			 input = externalinputs[i];
			 for (j = 1; j <= size; j++)
			 input += weights[j][i] * TempOutputs[j];
			 k4[i] = stepsize * Rtaus[i] * (input - TempStates[i]);
			 states[i] += (1.0/6.0)*k1[i] + (1.0/3.0)*k2[i] + (1.0/3.0)*k3[i] + (1.0/6.0)*k4[i];
			 outputs[i] = sigmoid(gains[i]*(states[i]+biases[i]));
			 }
			 }
			 
			 
			 */
		}
		
		sigmoid_type sigmoid; //<! Sigmoid for this neuron.
		double input; //!< Input to this neuron.
		double output; //!< Output from this neuron.
		double state; //!< State of this neuron.
		double tau; //!< Time constant for this neuron.
		double gain; //!< Gain for this neuron.
	};
	
	
	/*! Recurrent link.
	 */
	struct ctrnn_link {
		//! Default constructor.
		ctrnn_link(double w=1.0) : weight(w), t(0.0), t_minus1(0.0) { }
		
		double weight; //!< Weight of this link.
		double t; //!< Value of this link at time t (present).
		double t_minus1; //!< Value of this link at time t-1 (past).
	};
	
	
	//! Selector for CTRNNs.
	struct ctrnn_s { };
	
	
	//! Traits type for recurrent neural networks.
	template < >
	struct neural_network_traits<ctrnn_s> {
		typedef hyperbolic_tangent sigmoid_type;
		typedef ctrnn_neuron<sigmoid_type> neuron_type;
		typedef ctrnn_link link_type;
		typedef boost::adjacency_list<boost::setS,boost::vecS,boost::bidirectionalS,neuron_type,link_type> graph_type;
		
		static neuron_type make_input_neuron() { return neuron_type(neuron_base::INPUT); }
		static neuron_type make_hidden_neuron() { return neuron_type(neuron_base::HIDDEN); }
		static neuron_type make_output_neuron() { return neuron_type(neuron_base::OUTPUT); }
		static neuron_type make_inactive_neuron() { return neuron_type(neuron_base::INACTIVE); }
		static link_type make_link(double weight) { return link_type(weight); }
		
		//! Initializing constructor.
		neural_network_traits(double s) : _stepsize(s) {
		}
		
		double stepsize() const { return _stepsize; }
		double _stepsize; //!< Stepsize for this ctrnn.
	};
	
	
	//! Convenience typedef for CTRNNs.
	typedef neural_network<ctrnn_s> ctrnn;
	
} // nn

#endif
