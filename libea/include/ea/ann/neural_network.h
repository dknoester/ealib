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
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <ea/functional.h>
#include <ea/ann/sigmoid.h>
#include <ea/ann/filter.h>

namespace ealib {
    namespace bnu = boost::numeric::ublas;
    
    /*! "Simple" update method.
     
     For all nodes i:
     state(n_i)_t+1 = \sum_{j=in_edges} state(source(j)_t) * w_j
     */
    struct simple_update {
        //!  Update the neural network x: t+1 = t * A
        template <typename NeuralNetwork>
        void operator()(NeuralNetwork& N) {
            N._Tplus1 = bnu::prod(N._T, N._A);
        }
    };

    /*! CTRNN update method.
     
     \todo
     */
    struct ctrnn_update {
        template <typename NeuralNetwork>
        void operator()(NeuralNetwork& N) {
        }
    };

    
    /*! Artificial neural network.
     */
    template <
    typename Sigmoid=logistic,
    typename Filter=identity<double>,
    typename UpdateFunction=simple_update,
    typename StateVector=bnu::vector<double> >
    class neural_network {
    public:
        typedef Sigmoid sigmoid_type;
        typedef Filter filter_type;
        typedef filter_type input_conditioning_type;
        typedef unary_compose<filter_type,sigmoid_type> output_conditioning_type;
        typedef UpdateFunction update_function_type;
        typedef StateVector state_vector_type;
        typedef bnu::matrix<double> weight_matrix_type;
        typedef bnu::matrix_column<weight_matrix_type> weight_column_type;
        typedef bnu::matrix_row<weight_matrix_type> weight_row_type;

        //! Constructor.
        neural_network(std::size_t n) {
            resize(n);
        }
        
        //! Constructor.
        neural_network(std::size_t n, const filter_type& filt)
        : _fin(filt), _fout(filt,sigmoid_type()) {
            resize(n);
        }
        
        //! Resize this network.
        void resize(std::size_t n) {
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
        
        //! Retrieve the value of neuron i.
        double& operator[](std::size_t i) { return _T(i); }

        //! Retrieve the value of neuron i (const-qualified).
        const double& operator[](std::size_t i) const { return _T(i); }
        
        //! Set the state of neuron i to x (and apply input conditioning).
        void set(std::size_t i, double x) {
            _T(i) = _fin(x);
        }

        //! Update this network (produce outputs, assuming that inputs have been set).
        void update() {
            // update the network:
            _update(*this);
            // filter(sigmoid(t+1)):
            std::transform(_Tplus1.begin(), _Tplus1.end(), _Tplus1.begin(), _fout);
            // rotate t,t+1
            std::swap(_T,_Tplus1);
        }
        
    protected:
        // the various update methods need access to internals:
        friend struct simple_update;

        input_conditioning_type _fin; //!< Input conditioning function.
        output_conditioning_type _fout; //!< Output conditioning function.
        update_function_type _update; //!< Function object that updates this neural network.
        weight_matrix_type _A; //!< Adjacency matrix; a_ij == w(e_ij).
        state_vector_type _T; //!< State vector for time t.
        state_vector_type _Tplus1; //!< State vector for time t+1.
    };

} // network

#endif
