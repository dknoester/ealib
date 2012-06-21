/* hmm_network.h
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
#ifndef _FN_HMM_HMM_NETWORK_H_
#define _FN_HMM_HMM_NETWORK_H_

#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

#include <fn/hmm/hmm.h>
#include <fn/hmm/hmm_header.h>
#include <fn/hmm/hmm_instruments.h>
#include <fn/hmm/hmm_node.h>

namespace fn {
	namespace hmm {
        
        struct hmm_exception {
            hmm_exception() { }
            hmm_exception(const std::string& m) : msg(m) {	}
            std::string msg;
        };
        
        
		/*! Hidden Markov Model Network (HMM Network).
		 
		 This is an operational form of an HMM network, which is to say that it can
		 be constructed and run in-place.  This class also specifies the in-memory 
		 layout of the HMM network:
		 
		 Header, fixed size 4*int:
		 | num_inputs | num_outputs | num_hidden | num_states ...
		 
		 state vector, var. size states*int:
		 | inputs... | outputs... | hidden states...|
		 
		 state vector, var. size states*int:
		 | inputs... | outputs... | hidden states...|
		 
		 hidden node map, var. size num_hidden*int:
		 | offset... |
		 
		 node 0..n, var. size
		 .
		 .
		 .
		 
		 Rotating t-1 and t (which is done during each update) is simply a matter of 
		 swapping the offsets for the two state vectors.
		 
		 \todo Ideally, this would be independent of the concept of a genome...
		 */
		class hmm_network {
		public:
			typedef std::vector<boost::shared_ptr<hmm_node> > node_list; //!< List of hmm nodes.
			
			//! Constructor.
			hmm_network(const genome& g, int in, int out, int hidden);
			
			//! Destructor.
			virtual ~hmm_network();
            
            //! Cat the network to this one.
            hmm_network& operator+=(const hmm_network& that);
			
            //! Multiply this network.
            hmm_network& operator*=(std::size_t n);
            
			//! Update this network n times before copying the outputs.
			template <typename InputIterator, typename OutputIterator, typename RNG>
			void update_n(int n, InputIterator first, InputIterator last, OutputIterator result, RNG& rng, hmm_instrument* instr=0) {
                if(std::distance(first,last) != _h->nin) {
                    throw hmm_exception("number of inputs do not match this network");
                }

                for( ; n>0; --n) {
                    _h->rotate();
                    std::copy(first, last, _h->tminus1_begin());

                    if(instr) {
                        instr->upper_half(_h);
                    }
                    for(node_list::iterator i=_nodes.begin(); i!=_nodes.end(); ++i) {
                        (*i)->update(_h, rng(std::numeric_limits<int>::max()));
                    }
                    if(instr) {
                        instr->bottom_half(_h);
                    }
                }
				
				std::copy(_h->t_output_begin(), _h->t_output_end(), result);
			}
            
            //! Update this network n times, accumulating the outputs.
			template <typename InputIterator, typename OutputIterator, typename RNG>
			void accumulate_updates(int n, InputIterator first, InputIterator last, OutputIterator result, RNG& rng, hmm_instrument* instr=0) {
                if(std::distance(first,last) != _h->nin) {
                    throw hmm_exception("number of inputs do not match this network");
                }
                
                std::vector<int> outputs(_h->nout);
                
                for( ; n>0; --n) {
                    _h->rotate();
                    std::copy(first, last, _h->tminus1_begin());
                    
                    if(instr) {
                        instr->upper_half(_h);
                    }
                    for(node_list::iterator i=_nodes.begin(); i!=_nodes.end(); ++i) {
                        (*i)->update(_h, rng(std::numeric_limits<int>::max()));
                    }
                    if(instr) {
                        instr->bottom_half(_h);
                    }
                    
                    for(std::size_t i=0; i<_h->nout; ++i) {
                        outputs[i] += *(_h->t_output_begin()+i);
                    }
                }
				
				std::copy(outputs.begin(), outputs.end(), result);
			}

			
			//! Retrieve the number of HMM nodes in this network.
			inline std::size_t num_nodes() const { return _nodes.size(); }
			
			//! Return the number of states in this network.
			inline int num_states() const { return _h->nstates; }		
			
			//! Return the number of inputs to this network.
			inline int num_inputs() const { return _h->nin; }		
			
			//! Return the number of outputs from this network.
			inline int num_outputs() const { return _h->nout; }		
			
			//! Return the number of hidden states in this network.
			inline int num_hidden() const { return _h->nhidden; }		
			
			//! Return the header for this HMM network.
			inline const hmm_header* header() const { return _h; }
			
			//! Return node i.
			inline hmm_node* node(int i) { return _nodes[i].get(); }
			
			//! Return the offset of node i (debug).
			inline int onode(int i) { return _h->onode(i); }
			
			//! Clear the state of this HMM network.
			inline void clear(hmm_instrument* instr=0) { 
                if(instr) {
                    instr->clear();
                }
                _h->clear(); 
            }
            
		protected:
			int _mem_needed; //!< Number of bytes needed to store this network in memory.
			hmm_header* _h; //!< Header for the in-memory HMM network.
			boost::shared_array<char> _rdata; //!< Raw data used to store this HMM in memory.
			node_list _nodes; //!< List of nodes in the HMM.
		};
		
		
		//! Outputs the t and t-1 state vectors.
		std::ostream& operator<<(std::ostream& out, const hmm_network& h);
		
	} // hmm
} // fn

#endif
