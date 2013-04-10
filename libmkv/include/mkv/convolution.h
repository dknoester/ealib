/* convolve.h
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
#ifndef _MKV_CONVOLUTION_H_
#define _MKV_CONVOLUTION_H_

#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <algorithm>
#include <ea/algorithm.h>

namespace mkv {

    /*! Unary matrix function that returns the maximum value from Matrix M.
     */
    struct max_pool {
        template <typename Matrix>
        typename Matrix::value_type operator()(Matrix& M) {
            typename Matrix::value_type t=M(0,0);
            for(std::size_t i=0; i<M.size1(); ++i) {
                for(std::size_t j=0; j<M.size2(); ++j) {
                    t = std::max(t, M(i,j));
                }
            }
            return t;
        }
    };

    /*! Random access Matrix iterator, used to bridge Matrix input to a random
     access iterator suitable for Markov networks.
     */
    template <typename Matrix>
    struct ra_matrix_iterator {
        ra_matrix_iterator(Matrix& M) : _M(M) {
        }
        
        typename Matrix::value_type operator[](std::size_t i) {
            return _M(i/_M.size2(), i % _M.size2());
        }
        
        Matrix& _M;
    };

    /*! Unary matrix function that calls a Markov network on Matrix M.
     */
    template <typename Network>
    struct call_network {
        call_network(Network& net, int n) : _net(net), _n(n) {
        }
        
        template <typename Matrix>
        typename Matrix::value_type operator()(Matrix& M) {
            mkv::update(_net, _n, ra_matrix_iterator<Matrix>(M));
            return ealib::algorithm::range_pair2int(_net.begin_output(), _net.end_output());
        }
        
        Network& _net; //!< Network to be called on a matrix.
        int _n; //!< Number of times to update the network.
    };

    /*! 2-dimensional (r x c) convolution of unary matrix function F over matrix
     M, producing Matrix P.
     
     While r_skip and c_skip default to 1, they can be set to different values,
     e.g., in the case of max-pooling r_skip and c_skip should be set to r and c.
     */
    template <typename InputMatrix, typename OutputMatrix, typename UnaryMatrixFunction>
    void convolve2d(InputMatrix& M, std::size_t r, std::size_t c, UnaryMatrixFunction F, OutputMatrix& P, std::size_t r_skip=1, std::size_t c_skip=1) {
        using namespace boost::numeric::ublas;
        
        std::size_t ar = M.size1()-r+1; // number of possible anchor rows
        std::size_t ac = M.size2()-c+1; // number of possible anchor columns
        P.resize(ar/r_skip, ac/c_skip);
        
        for(std::size_t i=0; i<P.size1(); ++i) {
            for(std::size_t j=0; j<P.size2(); ++j) {
                std::size_t m=i*r_skip;
                std::size_t n=j*c_skip;
                matrix_range<InputMatrix> R(M, range(m,m+r), range(n,n+c));
                P(i,j) = F(R);
            }
        }
    }
    
} // mkv

#endif
