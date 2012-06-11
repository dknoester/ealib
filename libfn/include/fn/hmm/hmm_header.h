/* hmm_header.h
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
#ifndef _FN_HMM_HMM_HEADER_H_
#define _FN_HMM_HMM_HEADER_H_

#include <algorithm>

#include <fn/hmm/primitive.h>

namespace fn {
	namespace hmm {
		
		//! Common header for all node types.
		struct node_header {
			union {
				struct {
					int start_codon[2]; //!< The start codon for this node.
					int nin; //!< Number of inputs.
					int nout; //!< Number of outputs.
					int oin; //!< Offset of the inputs from the beginning of this node.
					int oout; //!< Offset of the outputs from the beginning of this node.
				};
				int _data[]; //!< All node data.
			};
			
			//! Return the index of the i'th input to this node.
			__GPU_DECL__ int& xinput(int i) { return _data[oin+i]; }
			
			//! Return the index of the i'th output from this node.
			__GPU_DECL__ int& xoutput(int i) { return _data[oout+i]; }
		};
		
		//! Header for table-based nodes.
		struct table_header : node_header {
			int ncol; //!< Number of columns in the table.
			int otable; //!< Offset of the table from the beginning of this node.
			
			//! Return entry (i,j) in this node's table.
			__GPU_DECL__ int& entry(int i, int j) { return _data[otable+i*ncol+j]; }
			
			//! Return a pointer to the i'th row in this table.
			__GPU_DECL__ int* row(int i) { return &_data[otable+i*ncol]; }
		};
		
		
		/*! Specifies the in-memory layout of an HMM network.
		 
		 #input | #outputs | #hidden | #total states
		 #nodes | nodemap offset 
		 t-1 offset | 
		 t offset |
		 <nodemap...> |
		 <state vector...> |
		 <state vector...> |
		 <nodes...>
         
         
         State vectors are arrayed like so:
         [inputs] [outputs] [hidden states]
		 */
		struct hmm_header {
			union {
				struct {
					int nin; //!< Number of inputs to this HMM.
					int nout; //!< Number of outputs from this HMM.
					int nhidden; //!< Number of hidden states.
					int nstates; //!< Total number of states (hidden+inputs+outputs).
					int nnodes; //!< Number of nodes in this HMM.
					int onodemap; //!< Offset into _data of the node map.
					int otminus1; //!< Offset into _data of the state vector for t-1.
					int ot; //!< Offset into _data of the state vector for t.
				};
				int _data[]; //!< Entire HMM data.
			};
			
			//! Rotate the state vectors.
			inline void rotate() { 
				std::swap(otminus1, ot);
                std::fill(t_begin(), t_end(), 0);
                // we want to clear t so that 1s don't fix in the state vector
                // std::fill(t_begin(), t_outputs(), 0);
                // std::fill(t_hidden(), t_end(), 0);
			}		
			
			//! Clear the state vectors.
			inline void clear() { std::fill(t_begin(), t_end(), 0); std::fill(tminus1_begin(), tminus1_end(), 0); }
			
			//! Return the offset of node i.
			__GPU_DECL__ int& onode(int i) { return _data[onodemap+i]; }
			
			//! Return the value of state i at time t-1.
			__GPU_DECL__ int& tminus1(int i) { return _data[otminus1+i]; }
			
			//! Return the value of state i at time t.
			__GPU_DECL__ int& t(int i) { return _data[ot+i]; }
			
			//! Return a pointer to the i'th node.
			__GPU_DECL__ node_header* node_ptr(int i) { return reinterpret_cast<node_header*>(&_data[onode(i)]); }
			
			//! Return the size of the header in bytes (includes state vectors, but not nodes).
			inline int header_size() const { return sizeof(hmm_header)+sizeof(int)*(nstates*2+nnodes); }
			
			//! Return a pointer to the beginning of the state vector for time t-1.
			inline int* tminus1_begin() { return &_data[otminus1]; }
			
			//! Return a pointer to the first output state for time t-1.
			inline int* tminus1_outputs() { return &_data[otminus1+nin]; }

            //! Return a pointer to the first hidden state for time t-1.
			inline int* tminus1_hidden() { return &_data[otminus1+nin+nout]; }

			//! Return a pointer to the end of the state vector for time t-1.
			inline int* tminus1_end() { return &_data[otminus1+nstates]; }
			
			//! Return a const pointer to the beginning of the state vector for time t-1.
			inline const int* tminus1_begin() const { return &_data[otminus1]; }
			
			//! Return a const pointer to the ending of the state vector for time t-1.
			inline const int* tminus1_end() const { return &_data[otminus1+nstates]; }
			
			//! Return a pointer to the beginning of the state vector for time t.
			inline int* t_begin() { return &_data[ot]; }

			//! Return a pointer to the first output state for time t.
			inline int* t_outputs() { return &_data[ot+nin]; }
			
			//! Return a pointer to the first hidden state for time t.
			inline int* t_hidden() { return &_data[ot+nin+nout]; }
			
			//! Return a pointer to the ending of the state vector for time t.
			inline int* t_end() { return &_data[ot+nstates]; }
			
			//! Return a const pointer to the beginning of the state vector for time t.
			inline const int* t_begin() const { return &_data[ot]; }
			
			//! Return a const pointer to the ending of the state vector for time t.
			inline const int* t_end() const { return &_data[ot+nstates]; }
			
			//! Return a pointer to the beginning of the outputs in the state vector for time t.
			inline int* t_output_begin() { return &_data[ot+nin]; }
			
			//! Return a pointer to the ending of the outputs in the state vector for time t.
			inline int* t_output_end() { return &_data[ot+nin+nout]; }
		};
		
	} // hmm
} // fn

#endif
