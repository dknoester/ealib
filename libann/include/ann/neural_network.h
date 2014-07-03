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
        typedef bnu::matrix<double> weight_matrix_type;
        typedef weight_matrix_type::array_type::iterator iterator;
        typedef weight_matrix_type::array_type::const_iterator const_iterator;
        typedef weight_matrix_type::array_type::reverse_iterator reverse_iterator;
        typedef weight_matrix_type::array_type::const_reverse_iterator const_reverse_iterator;
        
        //! Constructor.
        neural_network(std::size_t n) {
            resize(n);
        }
        
        //! Constructor.
        neural_network(std::size_t n, const filter_type& filt)
        : _fin(filt), _fout(filt,sigmoid_type()) {
            resize(n);
        }
        
        //! Constructor.
        template <typename ForwardIterator>
        neural_network(std::size_t n, ForwardIterator f) {
            resize(n);
            for(std::size_t i=0; i<n; ++i) {
                for(std::size_t j=0; j<n; ++j, ++f) {
                    _A(i,j) = *f;
                }
            }
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

        //! Returns a begin iterator to the weight matrix.
        iterator begin() { return _A.data().begin(); }
        
        //! Returns an end iterator to the weight matrix.
        iterator end() { return _A.data().end(); }
        
        //! Returns a begin iterator to the weight matrix (const-qualified).
        const_iterator begin() const { return _A.data().begin(); }
        
        //! Returns an end iterator to the weight matrix (const-qualified).
        const_iterator end() const { return _A.data().end(); }
        
        //! Returns a reverse begin iterator to the weight matrix.
        reverse_iterator rbegin() { return _A.data().rbegin(); }
        
        //! Returns a reverse end iterator to the weight matrix.
        reverse_iterator rend() { return _A.data().rend(); }
        
        //! Returns a reverse begin iterator to the weight matrix (const-qualified).
        const_reverse_iterator rbegin() const { return _A.data().rbegin(); }
        
        //! Returns a reverse end iterator to the weight matrix (const-qualified).
        const_reverse_iterator rend() const { return _A.data().rend(); }
        
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

        //! Update this network (produce outputs, assuming that inputs have been set).
        void update() {
            // update the network:
            _update(_Tplus1, _T, _A);
            // filter(sigmoid(t+1)):
            std::transform(_Tplus1.begin(), _Tplus1.end(), _Tplus1.begin(), _fout);
            // rotate t,t+1
            std::swap(_T,_Tplus1);
        }
        
        //! Serialization.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
        }

    protected:
        input_conditioning_type _fin; //!< Input conditioning function.
        output_conditioning_type _fout; //!< Output conditioning function.
        update_function_type _update; //!< Function object that updates this neural network.
        weight_matrix_type _A; //!< Adjacency matrix; a_ij == w(e_ij).
        state_vector_type _T; //!< State vector for time t.
        state_vector_type _Tplus1; //!< State vector for time t+1.
    };

} // network

#endif
