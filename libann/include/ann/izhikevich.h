/* izhikevich.h
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
#ifndef _ANN_IZHIKEVICH_H_
#define _ANN_IZHIKEVICH_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <ea/algorithm.h>
#include <ea/functional.h>
#include <ann/sigmoid.h>
#include <ann/filter.h>

namespace ann {
    namespace bnu = boost::numeric::ublas;
    
	/*! Neural network based on Izhikevich's model of spiking neurons.
     
     Drawn from Izhikevich's paper, "Simple Model of Spiking Neurons", 
     IEEE TEC, vol. 14, no. 6, Nov. 2003.
     
     Izhikevich neurons are updated according to the equations:
     v' = 0.04v^2 + 5v + 140 - u + I (1)
     u' = a(bv-u)                    (2)
     and
     if v >= 30 (mV), then v = c, u = u + d (3)
     
     v, u are dimensionless dependent variables
     I is a dimensionless independent variable
     a, b, c, d are dimensionless parameters
     t is time
     
     v: membrane potential of the neuron
     u: membrane recovery varible
     I: synaptic or injected dc-current (input / bias)
     a: time scale of u; smaller values == slower recovery
     b: relates subthreshold sensitity of u to v; greater values imply stronger coupling
     c: reset value of v after spiking
     d: effect spiking has on u at reset
     */
    class izhikevich {
    public:
		typedef bnu::vector<double> state_vector_type;
		typedef state_vector_type::iterator iterator;
		typedef state_vector_type::const_iterator const_iterator;
		typedef state_vector_type::reverse_iterator reverse_iterator;
		typedef state_vector_type::const_reverse_iterator const_reverse_iterator;
		typedef bnu::matrix<double> adj_matrix_type;
		
        //! Constructor.
        izhikevich(double dt // time step
                   , std::size_t nin
                   , std::size_t nout
                   , std::size_t nhid
                   , double a=0.02
                   , double b=0.2
                   , double c=-65.0
                   , double d=2.0
                   , double v0=-60.0
                   , double u0=12.0)
        : _dt(dt), _a(a), _b(b), _c(c), _d(d), _v0(v0), _u0(u0) {
			resize(nin, nout, nhid);
        }

        //! Constructor.
        template <typename ForwardIterator>
        izhikevich(double dt // time step
                   , std::size_t nin
                   , std::size_t nout
                   , std::size_t nhid
                   , ForwardIterator f
                   , double a=0.02
                   , double b=0.2
                   , double c=-65.0
                   , double d=2.0
                   , double v0=-60.0
                   , double u0=12.0)
        : _dt(dt), _a(a), _b(b), _c(c), _d(d), _v0(v0), _u0(u0) {
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
            _I = bnu::zero_vector<double>(n);
            _V = bnu::scalar_vector<double>(n, _v0);
            _U = bnu::scalar_vector<double>(n, _u0);
            _v140 = bnu::scalar_vector<double>(n, 140.0);
		}
		
		//! Clear this network.
		void clear() {
			_A.clear();
			_I.clear();
            _V.clear();
            _U.clear();
            _v140.clear();
		}
		
		//! Returns the size (number of neurons) in this neural network.
		std::size_t size() const {
			return _V.size();
		}
		
		//! Returns the weight between neuron i and j.
		double& operator()(std::size_t i, std::size_t j) {
			return _A(i,j);
		}
        
		//! Returns the activation level of neuron i at time t.
		double& operator[](std::size_t i) { return _V(i); }
		
		//! Returns the activation level of neuron i at time t (const-qualified).
		const double& operator[](std::size_t i) const { return _V(i); }
		
		//! Returns the input activation level of neuron i.
		double& input(std::size_t i) { return _I(i); }
		
		//! Returns the input activation level of neuron i (const-qualified).
		const double& input(std::size_t i) const { return _I(i); }
        
		//! Retrieve an iterator to the beginning of the inputs.
		iterator begin_input() { return _I.begin(); }
		
		//! Retrieve an iterator to the end of the inputs.
		iterator end_input() { return _I.begin() + _nin; }
		
		//! Retrieve an iterator to the beginning of the outputs.
		iterator begin_output() { return _V.begin() + _nin; }
		
		//! Retrieve an iterator to the end of the outputs.
		iterator end_output() { return _V.begin() + _nin + _nout; }
		
		/*! Update this network (assumes that inputs have been set).

         Izhikevich neurons are updated according to the equations:
         v' = 0.04v^2 + 5v + 140 - u + I (1)
         u' = a(bv-u)                    (2)
         and
         if v >= 30 (mV), then v = c, u = u + d (3)
         
         Here, I is both external and internal input; internal input is v * A,
         where v can be thought of as the current activation level of each
         neuron and A is the (weighted) adjacency matrix.
         */
		void update() {
            // update v and u:
            state_vector_type Vp1 = _dt * (0.04 * bnu::element_prod(_V, _V)
                                           + 5.0 * _V + _v140
                                           - _U + _I + bnu::prod(_V,_A));
            _U = _dt * _a * (_b*_V - _U);
            std::swap(_V, Vp1);
            
            // and take care of after-spike resetting:
            for(std::size_t i=0; i<_V.size(); ++i) {
                if(_V(i) >= 30.0) {
                    _V(i) = _c;
                    _U(i) = _U(i) + _d;
                }
            }
		}
		
		//! Updates the ANN n times given inputs [f,l) and time delta_t.
		template <typename ForwardIterator>
		void update(ForwardIterator f, ForwardIterator l, std::size_t n=1) {
			assert(std::distance(f,l)==_nin);
			std::copy(f, l, _I.begin());
			for( ; n>0; --n) {
				update();
			}
		}
		
	protected:
		double _dt; //!< Step size.
		std::size_t _nin, _nout, _nhid; //!< Number of inputs, outputs, and hidden neurons.
        double _a, _b, _c, _d, _v0, _u0; //!< Izhikevich neuron parameters.
		adj_matrix_type _A; //!< Adjacency matrix; a_ij == weight(e_ij).
		state_vector_type _I; //!< Input state vector.
		state_vector_type _V; //!< Membrane potentials
		state_vector_type _U; //!< Recovery potentials.
        state_vector_type _v140; //!< Scalar vector of 140.0.
        
	private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
			ar & boost::serialization::make_nvp("dt", _dt);
			ar & boost::serialization::make_nvp("nin", _nin);
			ar & boost::serialization::make_nvp("nout", _nout);
			ar & boost::serialization::make_nvp("nhid", _nhid);
            ar & boost::serialization::make_nvp("a", _a);
            ar & boost::serialization::make_nvp("b", _b);
            ar & boost::serialization::make_nvp("c", _c);
            ar & boost::serialization::make_nvp("d", _d);
            ar & boost::serialization::make_nvp("v0", _v0);
            ar & boost::serialization::make_nvp("u0", _u0);
			ar & boost::serialization::make_nvp("A", _A);
            ar & boost::serialization::make_nvp("I", _I);
            ar & boost::serialization::make_nvp("V", _V);
            ar & boost::serialization::make_nvp("U", _U);
        }
	};
	
} // ann

#endif
