/* prob_node.h
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
#ifndef _MKV_DETAIL_PROB_NODE_H_
#define _MKV_DETAIL_PROB_NODE_H_

#include <deque>
#include <ea/algorithm.h>
#include <mkv/detail/abstract_node.h>

namespace mkv {
    namespace detail {
        
        /*! Probabilistic Markov network node.
         */
        struct probabilistic_mkv_node : public abstract_markov_node {
            //! Constructor.
            template <typename ForwardIterator>
            probabilistic_mkv_node(index_list_type inputs, index_list_type outputs, ForwardIterator f, bool allow_zero) 
            : abstract_markov_node(inputs,outputs), _table(1<<inputs.size(), 1<<outputs.size()) {
                for(std::size_t i=0; i<_table.size1(); ++i) {
                    row_type row(_table, i);
                    f = ea::algorithm::normalize(f, f+_table.size2(), row.begin(), 1.0);
                }
            }
            
            /*! Reinforce the given output.
             
             The sign of the "scale" parameter determines if output (i,j) is reinforced (positive)
             or weakened (negative).
             */
            virtual void reinforce(std::size_t i, std::size_t j, double scale) {
                _table(i,j) *= 1.0 + scale;
                row_type row(_table, i);
                ea::algorithm::normalize(row.begin(), row.end(), row.begin(), 1.0);
            }
            
            //! Update the Markov network from this probabilistic node.
            void update(markov_network& mkv) {
                row_type row(_table, get_input(mkv));
                double p = mkv.rng().uniform_real(0.0,1.0);
                
                for(int i=0; i<static_cast<int>(_table.size2()); ++i) {
                    if(p <= row[i]) {
                        set_output(i, mkv);
                        return;
                    }
                    p -= row[i];
                }
                
                // if we get here, there was a floating point precision problem.
                // default to the final column:
                set_output(static_cast<int>(_table.size2()-1), mkv);
            }
            
            matrix_type _table; //!< Probability table.
        };
        
        
        /*! History-tracking probabilistic node.
         */
        struct probabilistic_history_mkv_node : public probabilistic_mkv_node {
            typedef std::deque<std::pair<std::size_t, std::size_t> > history_type;//!< Type for tracking history of decisions.
            
            //! Constructor.
            template <typename ForwardIterator>
            probabilistic_history_mkv_node(std::size_t hn,
                                           index_list_type inputs, index_list_type outputs, ForwardIterator ft, bool allow_zero) 
            : probabilistic_mkv_node(inputs, outputs, ft, allow_zero), _hn(hn) {
            }
            
            history_type::iterator begin() { return _history.begin(); }
            history_type::iterator end() { return _history.end(); }
            
            //! Reinforce all previous decisions by the given learning rate r.
            virtual void reinforce(double r) {
                for(std::size_t i=0; i<_history.size(); ++i) {
                    probabilistic_mkv_node::reinforce(_history[i].first, _history[i].second, r);
                }
            }
            
            //! Update the Markov network from this probabilistic node.
            void update(markov_network& mkv) {
                // prune history:
                while(_history.size() > (_hn-1)) {
                    _history.pop_front();
                }
                
                // output:
                std::size_t i = get_input(mkv);
                row_type row(_table, i);
                double p = mkv.rng().uniform_real(0.0,1.0);
                
                for(int j=0; j<static_cast<int>(_table.size2()); ++j) {
                    if(p <= row[j]) {
                        set_output(j, mkv);
                        _history.push_back(std::make_pair(i,j));
                        return;
                    }
                    p -= row[j];
                }
                
                // if we get here, there was a floating point precision problem.
                // default to the final column:
                set_output(static_cast<int>(_table.size2()-1), mkv);
                _history.push_back(std::make_pair(i,static_cast<int>(_table.size2()-1)));
            }
            
            std::size_t _hn; //!< Size of history to keep.
            history_type _history; //!< History of decisions made by this node.
        };
        
    } // detail
} /// mkv

#endif
