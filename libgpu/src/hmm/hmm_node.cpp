/* hmm_node.cpp
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
#include <fn/hmm/hmm_node.h>
#include <fn/hmm/hmm_header.h>
#include <fn/hmm/probabilistic_node.h>
#include <fn/hmm/deterministic_node.h>

/*! Returns true if position i of genome g is a start codon, and if so, how 
 much memory is required for this node, false otherwise. */
std::pair<bool,int> fn::hmm::hmm_node::start_codon(const genome& g, int start) {
	unsigned int codon=g[start];
	if((codon>=_FIRST) && (codon<=_LAST) && (g[start+1]==(255-codon))) {
		switch(codon) {
			case PROBABILISTIC: {
				return std::make_pair(true, detail::probabilistic_node::nodesize(g,start));
			}
			case DETERMINISTIC: {
				return std::make_pair(true, detail::deterministic_node::nodesize(g,start));
				break;
			}			
			default:
				assert(false);
		}
	}
	return std::make_pair(false,0);
}


/*! Build an HMM node based on the start codon i.
 */
fn::hmm::hmm_node* fn::hmm::hmm_node::build_node(const genome& g, int start, int* where) {
	switch(g[start]) {
		case PROBABILISTIC: {
			return new detail::probabilistic_node(g,start,where);
		}
		case DETERMINISTIC: {
			return new detail::deterministic_node(g,start,where);
			break;
		}			
		default: 
			assert(false);
	}	
	return 0;
}




/*! Rebase this node's inputs and outputs.
 
 This method adjusts this nodes' input and output offsets by the given input, output
 and hidden node offsets.  In this case, hdr refers to the **old** geometry, but it's
 needed to determine which of the new offsets are used.
 */
void fn::hmm::hmm_node::rebase(hmm_header* hdr, int oin, int oout, int ohid) {
    for(int i=0; i<num_inputs(); ++i) {
        int& x=xinput(i);
        if(x < hdr->nin) {
            x += oin;
        } else if(x < (hdr->nin + hdr->nout)) {
            x += oout;
        } else {
            x += ohid;
        }
    }
    for(int i=0; i<num_outputs(); ++i) {
        int& x=xoutput(i);
        if(x < hdr->nin) {
            x += oin;
        } else if(x < (hdr->nin + hdr->nout)) {
            x += oout;
        } else {
            x += ohid;
        }
    }
}
