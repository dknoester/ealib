/* markov_network.h
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
#ifndef _MKV_DETAIL_ADAPTIVE_PROB_NODE_H_
#define _MKV_DETAIL_ADAPTIVE_PROB_NODE_H_

#include <ea/algorithm.h>
#include <mkv/markov_network.h>
#include <mkv/detail/prob_node.h>

namespace mkv {
    namespace detail {
        
        /*! Synaptically-learning probabilistic Markov network node.
         */
        struct synprob_mkv_node : public probabilistic_history_mkv_node {
            //! Constructor.
            template <typename ForwardIterator>
            synprob_mkv_node(std::size_t hn,
                             std::size_t posf, weight_vector_type poswv, 
                             std::size_t negf, weight_vector_type negwv,
                             index_list_type inputs, index_list_type outputs, ForwardIterator ft, bool allow_zero) 
            : probabilistic_history_mkv_node(hn, inputs, outputs, ft, allow_zero), _posf(posf), _poswv(poswv), _negf(negf), _negwv(negwv) {
            }
            
            //! Learn.
            void learn(markov_network& mkv) {
                // learn:
                if(mkv.svm().state_tminus1(_posf)) {
                    // positive feedback
                    for(std::size_t i=0; (i<_poswv.size()) && (i<_history.size()); ++i) {
                        probabilistic_mkv_node::reinforce(_history[i].first, _history[i].second, _poswv[i]);
                    }
                }
                if(mkv.svm().state_tminus1(_negf)) {
                    // negative feedback
                    for(std::size_t i=0; (i<_negwv.size()) && (i<_history.size()); ++i) {
                        probabilistic_mkv_node::reinforce(_history[i].first, _history[i].second, _negwv[i]);
                    }
                }
            }
            
            //! Update the Markov network from this probabilistic node.
            void update(markov_network& mkv) {
                probabilistic_history_mkv_node::update(mkv);
                learn(mkv);
            }
            
            std::size_t _posf; //!< Index of positive feedback state.
            weight_vector_type _poswv; //!< Positive feedback weight vector.
            std::size_t _negf; //!< Index of negative feedback state.
            weight_vector_type _negwv; //!< Negative feedback weight vector.
        };
        
    } // detail
} // mkv

#endif
