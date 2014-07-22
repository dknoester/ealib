/* basic_neural_network.h
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
#ifndef _ANN_BASIC_NEURAL_NETWORK_H_
#define _ANN_BASIC_NEURAL_NETWORK_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/serialization/nvp.hpp>
#include <algorithm>

#include <ann/sigmoid.h>
#include <ann/filter.h>

namespace ann {
    namespace bnu = boost::numeric::ublas;
    
	
	/*! A basic neural network class.
	 */
	template
    < typename Sigmoid=logistic
    > class basic_neural_network {
    public:
		typedef Sigmoid sigmoid_type;
		typedef bnu::vector<double> state_vector_type;
		typedef state_vector_type::iterator iterator;
		typedef state_vector_type::const_iterator const_iterator;
		typedef state_vector_type::reverse_iterator reverse_iterator;
		typedef state_vector_type::const_reverse_iterator const_reverse_iterator;
		typedef bnu::matrix<double> adj_matrix_type;
		
        //! Constructor.
        basic_neural_network(std::size_t nin=0, std::size_t nout=0, std::size_t nhid=0) {
			resize(nin, nout, nhid);
        }
        
        //! Constructor that initializes the adjacency matrix via an iterator.
        template <typename ForwardIterator>
        basic_neural_network(std::size_t nin, std::size_t nout, std::size_t nhid, ForwardIterator f) {
			resize(nin, nout, nhid);
			for(std::size_t i=0; i<_A.size1(); ++i) {
				for(std::size_t j=0; j<_A.size2(); ++j, ++f) {
					_A(i,j) = *f;
				}
			}
        }
        
		//! Resize this network.
		void resize(std::size_t nin, std::size_t nout, std::size_t nhid) {
			_nin = nin;
			_nout = nout;
			_nhid = nhid;
			std::size_t n = _nin + _nout + _nhid;
			_A = bnu::zero_matrix<double>(n,n);
			_Y = bnu::zero_vector<double>(n);
		}
		
		//! Clear this network.
		void clear() {
			_A.clear();
			_Y.clear();
		}
		
		//! Returns the size (number of neurons) in this neural network.
		std::size_t size() const {
			return _Y.size();
		}
		
		//! Returns the weight between neuron i and j.
		double& operator()(std::size_t i, std::size_t j) {
			return _A(i,j);
		}
        
        //! Returns the index of input neuron i.
        std::size_t input(std::size_t i) {
            return i;
        }
        
        //! Returns the index of output neuron i.
        std::size_t output(std::size_t i) {
            return _nin + i;
        }

        //! Returns the index of hidden neuron i.
        std::size_t hidden(std::size_t i) {
            return _nin + _nout + i;
        }
		
		//! Returns the activation level of neuron i at time t.
		double& operator[](std::size_t i) { return _Y(i); }
		
		//! Returns the activation level of neuron i at time t (const-qualified).
		const double& operator[](std::size_t i) const { return _Y(i); }
		
		//! Retrieve an iterator to the beginning of the inputs.
		iterator begin_input() { return _Y.begin(); }
		
		//! Retrieve an iterator to the end of the inputs.
		iterator end_input() { return _Y.begin() + _nin; }
		
		//! Retrieve an iterator to the beginning of the outputs.
		iterator begin_output() { return _Y.begin() + _nin; }
		
		//! Retrieve an iterator to the end of the outputs.
		iterator end_output() { return _Y.begin() + _nin + _nout; }
		
		//! Update this network (assumes that inputs have been set).
		void update() {
			_Y = bnu::prod(_Y, _A);
			std::transform(_Y.begin(), _Y.end(), _Y.begin(), _sig);
		}
		
		//! Updates the ANN n times given inputs [f,l).
		template <typename ForwardIterator>
		void update(ForwardIterator f, ForwardIterator l, std::size_t n=1) {
			assert(static_cast<std::size_t>(std::distance(f,l))==_nin);
			std::copy(f, l, _Y.begin());
			for( ; n>0; --n) {
				update();
			}
		}
        
        /*! Update the weights in the adjacency matrix via Hebb's rule with decay.
         
         w_ij(t+1) = w_ij(t) + \eta * x_i(t) * x_j(t) - \gamma * (1.0 - x_i(t) * x_j(t))
         */
        void hebbian_update(double eta, double gamma) {
            for(std::size_t i=0; i<_A.size1(); ++i) {
                for(std::size_t j=0; j<_A.size2(); ++j) {
                    double x = _Y(i) * _Y(j);
                    _A(i,j) = _A(i,j) + eta * x - gamma * (1.0 - x);
                }
            }
        }
        
        /*! Update the weights in the adjacency matrix via Oja's rule.
         
         w_ij(t+1) = w_ij(t) + \eta * x_j(t) * (x_i(t) - w_ij(t)*x_j(t))
         */
        void oja_update(double eta) {
            for(std::size_t i=0; i<_A.size1(); ++i) {
                for(std::size_t j=0; j<_A.size2(); ++j) {
                    _A(i,j) = _A(i,j) + eta * _Y(j) * (_Y(i) - _A(i,j) * _Y(j));
                }
            }
        }
		
	protected:
		std::size_t _nin, _nout, _nhid; //!< Number of inputs, outputs, and hidden neurons.
		sigmoid_type _sig; //!< Sigmoid type.
		adj_matrix_type _A; //!< Adjacency matrix; a_ij == weight(e_ij).
		state_vector_type _Y; //!< Activation level state vector.
		
	private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
			ar & boost::serialization::make_nvp("ninputs", _nin);
			ar & boost::serialization::make_nvp("noutputs", _nout);
			ar & boost::serialization::make_nvp("nhidden", _nhid);
			ar & boost::serialization::make_nvp("weights", _A);
			ar & boost::serialization::make_nvp("activation_levels", _Y);
        }
	};
	
} // ann

#endif
