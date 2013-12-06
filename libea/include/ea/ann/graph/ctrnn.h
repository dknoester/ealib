/* ctrnn.h
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
#ifndef _ANN_CTRNN_H_
#define _ANN_CTRNN_H_

#include <ea/ann/graph/neural_network.h>
#include <ea/ann/graph/sigmoid.h>


namespace ann {
	
	/*! CTRNN neuron.
	 */
	template <typename Sigmoid=hyperbolic_tangent>
	struct ctrnn_neuron : public abstract_neuron {
		typedef Sigmoid sigmoid_type; //!< Sigmoid type, used for activation.

        //! Synapse type for ctrnn neurons.
        struct synapse_type {
            //! Default constructor.
            synapse_type(double w=0.0) : weight(w), t(0.0), t_minus1(0.0) { }
            
            double weight; //!< Weight of this link.
            double t; //!< Value of this link at time t (present).
            double t_minus1; //!< Value of this link at time t-1 (past).
        };
        
        
		//! Constructor.
		ctrnn_neuron() : abstract_neuron(), state(0.0), tau(1.0), gain(1.0) { }
				
		/*! Activation function for a CTRNN neuron.
		 
		 Activation of a CTRNN is based on the following equation:		 
		 */
		template <typename Vertex, typename Graph, typename Filter>
		void activate(Vertex v, Graph& g, Filter& filt) {
			// for all incoming edges of this neuron, sum the link weights * link value(t-1):
			typename Graph::in_edge_iterator ei, ei_end;
			input = 0.0;
			for(boost::tie(ei,ei_end)=boost::in_edges(v,g); ei!=ei_end; ++ei) {
				input += g[*ei].weight * g[*ei].t_minus1;
				g[*ei].t_minus1 = g[*ei].t; // rotate present->past once it's been used.
			}
			
			// update the state of this neuron:
			state += delta_t * tau * (input - state);
			
			// update the outgoing edges with this neuron's output:
			output = filt(sigmoid(gain * state));

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
		double state; //!< State of this neuron.
		double tau; //!< Time constant for this neuron.
		double gain; //!< Gain for this neuron.
	};
	
} // ann

#endif
