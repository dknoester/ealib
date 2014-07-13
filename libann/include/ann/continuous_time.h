/* continuous_time.h
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
#ifndef _ANN_CONTINUOUS_TIME_H_
#define _ANN_CONTINUOUS_TIME_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <ea/algorithm.h>
#include <ea/functional.h>
#include <ann/sigmoid.h>
#include <ann/filter.h>

namespace ann {
    namespace bnu = boost::numeric::ublas;
    
	//! Selector for single step integration.
	struct singleStepS { };

	//! Selector for RK4 integration.
	struct rk4stepS{ };
	
	/*! Continuous-time Recurrent Neural Network.
	 */
	template
    < typename Sigmoid=logistic
	, typename StepFunctionTag=rk4stepS
    > class continuous_time {
    public:
		typedef Sigmoid sigmoid_type;
		typedef bnu::vector<double> state_vector_type;
		typedef state_vector_type::iterator iterator;
		typedef state_vector_type::const_iterator const_iterator;
		typedef state_vector_type::reverse_iterator reverse_iterator;
		typedef state_vector_type::const_reverse_iterator const_reverse_iterator;
		typedef bnu::matrix<double> adj_matrix_type;
		typedef StepFunctionTag step_function_tag;
		
        //! Constructor.
        continuous_time(double dt, std::size_t nin, std::size_t nout, std::size_t nhid) : _delta_t(dt) {
			resize(nin, nout, nhid);
        }
        
        //! Constructor.
        template <typename ForwardIterator>
        continuous_time(double dt, std::size_t nin, std::size_t nout, std::size_t nhid, ForwardIterator f) : _delta_t(dt) {
			resize(nin, nout, nhid);
			for(std::size_t i=0; i<_A.size1(); ++i) {
				for(std::size_t j=0; j<_A.size2(); ++j, ++f) {
					_A(i,j) = *f;
				}
			}
			for(std::size_t i=0; i<_A.size1(); ++i) {
				_tau[i] = *f++;
				_gain[i] = *f++;
				_bias[i] = *f++;
			}
        }
		
		//! Resize this network.
		void resize(std::size_t nin, std::size_t nout, std::size_t nhid) {
			_nin = nin;
			_nout = nout;
			_nhid = nhid;
			std::size_t n = _nin + _nout + _nhid;
			_A = bnu::zero_matrix<double>(n,n);
			_X = _Y = _S = _tau = _gain = _bias = bnu::zero_vector<double>(n);
		}
		
		//! Clear this network.
		void clear() {
			_A.clear();
			_X.clear();
			_Y.clear();
			_S.clear();
			_tau.clear();
			_gain.clear();
			_bias.clear();
		}
		
		//! Returns the size (number of neurons) in this neural network.
		std::size_t size() const {
			return _Y.size();
		}
		
		//! Returns the weight between neuron i and j.
		double& operator()(std::size_t i, std::size_t j) {
			return _A(i,j);
		}

		//! Returns the activation level of neuron i at time t.
		double& operator[](std::size_t i) { return _Y(i); }
		
		//! Returns the activation level of neuron i at time t (const-qualified).
		const double& operator[](std::size_t i) const { return _Y(i); }
		
		//! Returns the input activation level of neuron i.
		double& input(std::size_t i) { return _X(i); }
		
		//! Returns the input activation level of neuron i (const-qualified).
		const double& input(std::size_t i) const { return _X(i); }

		//! Retrieve an iterator to the beginning of the inputs.
		iterator begin_input() { return _X.begin(); }
		
		//! Retrieve an iterator to the end of the inputs.
		iterator end_input() { return _X.begin() + _nin; }
		
		//! Retrieve an iterator to the beginning of the outputs.
		iterator begin_output() { return _Y.begin() + _nin; }
		
		//! Retrieve an iterator to the end of the outputs.
		iterator end_output() { return _Y.begin() + _nin + _nout; }
		
		//! Update this network (assumes that inputs have been set).
		void update() {
			update(step_function_tag());
		}
		
		//! Updates the ANN n times given inputs [f,l) and time delta_t.
		template <typename ForwardIterator>
		void update(ForwardIterator f, ForwardIterator l, std::size_t n=1) {
			assert(std::distance(f,l)==_nin);
			std::copy(f, l, _X.begin());
			for( ; n>0; --n) {
				update(step_function_tag());
			}
		}
		
	protected:
		//! Returns vector v with each element having been passed through the sigmoid.
		inline state_vector_type sigmoid(const state_vector_type& v) {
			state_vector_type r(v.size());
			std::transform(v.begin(), v.end(), r.begin(), _sig);
			return r;
		}
		
		//! Non-integrated single step update.
		void update(singleStepS) {
			_X += bnu::prod(_Y, _A);
			_S += _delta_t * bnu::element_prod(_tau, _X-_S);
			_Y = sigmoid(bnu::element_prod(_gain, _S+_bias));
		}
		
		//! RK4 integrated step update.
		void update(rk4stepS) {
			// T_s and T_o are temporary states and outputs, respectively.
			// first step:
			state_vector_type X = _X + bnu::prod(_Y, _A);
			state_vector_type K1 = _delta_t * bnu::element_prod(_tau, X-_S);
			state_vector_type T_s = _S + K1/2.0;
			state_vector_type T_o = sigmoid(bnu::element_prod(_gain, T_s+_bias));
			
			// second step:
			X = _X + bnu::prod(T_o, _A);
			state_vector_type K2 = _delta_t * bnu::element_prod(_tau, X-T_s);
			T_s = _S + K2/2.0;
			T_o = sigmoid(bnu::element_prod(_gain, T_s+_bias));
			
			// third step:
			X = _X + bnu::prod(T_o, _A);
			state_vector_type K3 = _delta_t * bnu::element_prod(_tau, X-T_s);
			T_s = _S + K2;
			T_o = sigmoid(bnu::element_prod(_gain, T_s+_bias));
			
			// fourth step:
			X = _X + bnu::prod(T_o, _A);
			state_vector_type K4 = _delta_t * bnu::element_prod(_tau, X-T_s);
			_S += (K1+K4)/6.0 + (K2+K3)/3.0;
			_Y = sigmoid(bnu::element_prod(_gain, _S+_bias));
		}
		
		double _delta_t; //!< Step size for this CTRNN.
		std::size_t _nin, _nout, _nhid; //!< Number of inputs, outputs, and hidden neurons.
		sigmoid_type _sig; //!< Sigmoid type.
		adj_matrix_type _A; //!< Adjacency matrix; a_ij == weight(e_ij).
		state_vector_type _X; //!< Input state vector.
		state_vector_type _Y; //!< Activation levels.
		state_vector_type _S; //!< Neuron state vector.
		state_vector_type _tau; //!< Time constants.
		state_vector_type _gain; //!< Gains.
		state_vector_type _bias; //!< Biases.

	private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
			ar & boost::serialization::make_nvp("delta_t", _delta_t);
			ar & boost::serialization::make_nvp("ninputs", _nin);
			ar & boost::serialization::make_nvp("noutputs", _nout);
			ar & boost::serialization::make_nvp("nhidden", _nhid);
			ar & boost::serialization::make_nvp("weights", _A);
			ar & boost::serialization::make_nvp("inputs", _X);
			ar & boost::serialization::make_nvp("activation_levels", _Y);
			ar & boost::serialization::make_nvp("states", _S);
			ar & boost::serialization::make_nvp("tau", _tau);
			ar & boost::serialization::make_nvp("gain", _gain);
			ar & boost::serialization::make_nvp("bias", _bias);
        }
	};
	
} // ann

#endif
