/* neural_network.h
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
#ifndef _EA_NEURAL_NETWORK_H_
#define _EA_NEURAL_NETWORK_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <ea/algorithm.h>
#include <ea/functional.h>

namespace ann {
    namespace bnu = boost::numeric::ublas;
    
    /*! Generic artificial neural network class.
     */
    template
    < typename Sigmoid
    , typename Filter
    , typename UpdateFunction
    > class neural_network {
    public:
        typedef Sigmoid sigmoid_type;
        typedef Filter filter_type;
        typedef filter_type input_conditioning_type;
        typedef ealib::unary_compose<filter_type,sigmoid_type> output_conditioning_type;
        typedef UpdateFunction update_function_type;
        typedef bnu::vector<double> state_vector_type;
        typedef state_vector_type::iterator iterator;
        typedef state_vector_type::const_iterator const_iterator;
        typedef state_vector_type::reverse_iterator reverse_iterator;
        typedef state_vector_type::const_reverse_iterator const_reverse_iterator;
        typedef bnu::matrix<double> weight_matrix_type;
        
        //! Constructor.
        neural_network(std::size_t nin, std::size_t nout, std::size_t nhid) {
            resize(nin, nout, nhid);
        }
        
        //! Constructor.
        neural_network(std::size_t nin, std::size_t nout, std::size_t nhid, const filter_type& filt)
        : _fin(filt), _fout(filt,sigmoid_type()) {
            resize(nin, nout, nhid);
        }
        
        //! Constructor.
        template <typename ForwardIterator>
        neural_network(std::size_t nin, std::size_t nout, std::size_t nhid, ForwardIterator f) {
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
            _A = bnu::scalar_matrix<double>(n,n,0.0);
            _T = bnu::scalar_vector<double>(n,0.0);
            _Tplus1 = bnu::scalar_vector<double>(n,0.0);
        }
        
        //! Clear this network.
        void clear() {
            _A.clear();
            _T.clear();
            _Tplus1.clear();
        }
        
        //! Link neuron_i -> neuron_j with weight w.
        double& link(std::size_t i, std::size_t j) {
            return _A(i,j);
        }
        
        //! Returns the state of neuron i at time t.
        double& operator[](std::size_t i) { return _T(i); }

        //! Returns the state of neuron i at time t (const-qualified).
        const double& operator[](std::size_t i) const { return _T(i); }
        
        //! Set the state of neuron i at time t to x (and apply input conditioning).
        void set(std::size_t i, double x) {
            _T(i) = _fin(x);
        }

        //! Retrieve an iterator to the beginning of the inputs.
        iterator begin_input() { return _T.begin(); }
        
        //! Retrieve an iterator to the end of the inputs.
        iterator end_input() { return _T.begin() + _nin; }
        
        //! Retrieve an iterator to the beginning of the outputs.
        iterator begin_output() { return _T.begin() + _nin; }
        
        //! Retrieve an iterator to the end of the outputs.
        iterator end_output() { return _T.begin() + _nin + _nout; }
        
        //! Retrieve an iterator to the beginning of the hidden states.
        iterator begin_hidden() { return _T.begin() + _nin + _nout; }
        
        //! Retrieve an iterator to the end of the hidden states.
        iterator end_hidden() { return _T.end(); }
        
        //! Update this network (assumes that inputs have been set).
        void update() {
            // update the network:
            _update(_Tplus1, _T, _A);
            // filter(sigmoid(t+1)):
            std::transform(_Tplus1.begin(), _Tplus1.end(), _Tplus1.begin(), _fout);
            // rotate t,t+1
            std::swap(_T,_Tplus1);
        }
        
        //! Updates the ANN n times given inputs [f,l).
        template <typename ForwardIterator>
        void update(ForwardIterator f, ForwardIterator l, std::size_t n=1) {
            assert(std::distance(f,l)==_nin);
            for( ; n>0; --n) {
                // apply input conditioning and copy the inputs into our state vector (req'd for the matrix math):
                std::transform(f, l, _T.begin(), _fin);
                update();
            }
        }

    protected:
        std::size_t _nin, _nout, _nhid; //!< Number of inputs, outputs, and hidden neurons.
        input_conditioning_type _fin; //!< Input conditioning function.
        output_conditioning_type _fout; //!< Output conditioning function.
        update_function_type _update; //!< Function object that updates this neural network.
        weight_matrix_type _A; //!< Adjacency matrix; a_ij == w(e_ij).
        state_vector_type _T; //!< State vector for time t.
        state_vector_type _Tplus1; //!< State vector for time t+1.
    };

} // network

#endif
