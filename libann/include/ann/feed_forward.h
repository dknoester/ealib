/* feed_forward_neural_network.h
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

#include <ann/sigmoid.h>
#include <ann/filter.h>
#include <ann/neural_network.h>

namespace ann {
    namespace bnu = boost::numeric::ublas;
    
    /*! Feed-forward update method.
     
     For all nodes i:
     state(n_i)_t+1 = \sum_{j=in_edges} state(source(j)_t) * w_j
     
     Update the neural network x: t+1 = t * A

     */
    struct feed_forward_update {
        template <typename StateVector, typename AdjacencyMatrix>
        void operator()(StateVector& tp1, StateVector& t, AdjacencyMatrix& A) {
            tp1 = bnu::prod(t, A);
        }
    };

    /*! Feed-forward artificial neural network definition.
     */
    template
    < typename Sigmoid=logistic
    , typename Filter=identity<double>
    > class feed_forward
    : public neural_network
    < Sigmoid
    , Filter
    , feed_forward_update
    > {
    public:
        typedef neural_network<Sigmoid,Filter,feed_forward_update> parent;

        //! Constructor.
        feed_forward(std::size_t n) : parent(n) {
        }
        
        //! Constructor.
        feed_forward(std::size_t n, const Filter& filt) : parent(n, filt) {
        }
        
        //! Constructor.
        template <typename ForwardIterator>
        feed_forward(std::size_t n, ForwardIterator f) : parent(n, f) {
        }
    };
    
} // ann

#endif
