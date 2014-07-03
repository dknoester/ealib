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
#ifndef _EA_FEED_FORWARD_H_
#define _EA_FEED_FORWARD_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <ann/sigmoid.h>
#include <ann/filter.h>
#include <ann/neural_network.h>

namespace ann {
    namespace bnu = boost::numeric::ublas;
    
    
    /*! CTRNN update method.
     */
    struct ctrnn_update {
        template <typename StateVector, typename AdjacencyMatrix>
        void operator()(StateVector& tp1, StateVector& t, AdjacencyMatrix& A) {
        }

        
        // f(t,y) == t * tau * (bnu::prod(y, A) - s)
        template <typename StateVector, typename AdjacencyMatrix>
        StateVector _f(double h,
                       StateVector& k,
                       StateVector& y_n,
                       StateVector& tau,
                       AdjacencyMatrix& A) {
            return sigmoid(h * bnu::element_prod(tau, bnu::prod(y_n+k,A) - y_n));
        }
        
        template <typename StateVector, typename UpdateFunction>
        void operator()(StateVector& y_np1,
                        double h,
                        StateVector& y_n,
                        StateVector& tau,
                        AdjacencyMatrix& A) {
            k1 = _f(h, bnu::zero_vector<double>(y_n.size()), y_n, tau, A);
            k2 = _f(h/2.0, k1, y_n, tau, A);
            k3 = _f(h/2.0, k2, y_n, tau, A);
            k4 = _f(h, k3, y_n, tau, A);
            y_np1 = y_n + h/6.0*(k1 + 2.0*k2 + 2.0*k3 + k4);
        }
    };
    
    
    /*! Continuous-time recurrent artificial neural network definition.
     */
    template
    < typename Sigmoid=logistic
    , typename Filter=identity<double>
    > class ctrnn
    : public neural_network
    < Sigmoid
    , Filter
    , ctrnn_update
    > {
    public:
        typedef neural_network<Sigmoid,Filter,ctrnn_update> parent;
        
        //! Constructor.
        ctrnn(std::size_t n) : parent(n) {
        }
        
        //! Constructor.
        ctrnn(std::size_t n, const Filter& filt) : parent(n, filt) {
        }
    };

} // ann

#endif
