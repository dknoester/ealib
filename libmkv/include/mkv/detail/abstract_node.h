/* abstract_node.h
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
#ifndef _MKV_DETAIL_ABSTRACT_NODE_H_
#define _MKV_DETAIL_ABSTRACT_NODE_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <vector>
#include <mkv/markov_network.h>

namespace mkv {
    namespace detail {
        
        typedef std::vector<std::size_t> index_list_type; //!< Type for a list of indices.
        typedef boost::numeric::ublas::matrix<double> matrix_type;
        typedef boost::numeric::ublas::matrix_column<matrix_type> column_type;
        typedef boost::numeric::ublas::matrix_row<matrix_type> row_type;
        typedef std::vector<double> weight_vector_type; //!< Type for feedback weights vector.
        
        //! Abstract base class for nodes in a Markov network.
        struct abstract_markov_node {
            abstract_markov_node(index_list_type inputs, index_list_type outputs)
            : _in(inputs), _out(outputs) {
            }
            
            //! Destructor.
            virtual ~abstract_markov_node() { }
            
            //! Update the Markov network from this node.
            virtual void update(markov_network& mkv) = 0;
            
            //! Retrieve the input to this node from the Markov network's state machine at time t-1.
            int get_input(markov_network& mkv) {
                int x=0;
                for(std::size_t i=0; i<_in.size(); ++i) {
                    x |= (mkv.svm().state_tminus1(_in[i]) & 0x01) << (_in.size()-1-i);
                }
                return x;
            }
            
            //! Set the output from this node to the Markov network's state machine at time t.
            void set_output(int x, markov_network& mkv) {
                for(std::size_t i=0; i<_out.size(); ++i) {
                    mkv.svm().state_t(_out[i]) |= (x >> (_out.size()-1-i)) & 0x01;
                }
            }
            
            index_list_type _in; //!< Input state indices to this node.
            index_list_type _out; //!< Output state indices from this node.
        };
                
    } // detail
} // mkv

#endif
