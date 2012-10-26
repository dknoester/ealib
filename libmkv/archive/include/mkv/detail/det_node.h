/* det_node.h
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
#ifndef _MKV_DETAIL_DET_NODE_H_
#define _MKV_DETAIL_DET_NODE_H_

#include <mkv/markov_network.h>
#include <mkv/detail/abstract_node.h>

namespace mkv {
    namespace detail {
        
        /*! Deterministic Markov network node.
         */
        struct deterministic_mkv_node : public abstract_markov_node {
            //! Constructor.
            template <typename ForwardIterator>
            deterministic_mkv_node(index_list_type inputs, index_list_type outputs, ForwardIterator f) 
            : abstract_markov_node(inputs,outputs), _table(1<<_in.size()) {
                for(std::size_t i=0; i<static_cast<std::size_t>(1<<_in.size()); ++i, ++f) {
                    _table[i] = *f;
                }
            }

            //! Return a string suitable for graphviz output.
            virtual std::string graphviz();

            //! Update the Markov network from this deterministic node.
            void update(markov_network& mkv) {
                set_output(_table[get_input(mkv)], mkv);
            }
            
            index_list_type _table; //!< Deterministic table.
        };
        
    } // detail
} // mkv

#endif
