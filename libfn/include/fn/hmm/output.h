/* output.h
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
#ifndef _FN_HMM_OUTPUT_H_
#define _FN_HMM_OUTPUT_H_

#include <fn/hmm/hmm_header.h>

namespace fn {
	namespace hmm {
		
		/*! Set the outputs of the HMM based on the given node.
		 */
		__GPU_DECL__ void set_outputs(const int x, hmm_header* hdr, node_header* ndr) {
			for(int i=0; i<ndr->nout; ++i) {
				int no=ndr->xoutput(i);
				hdr->t(no) |= (x >> (ndr->nout-1-i)) & 0x01;
			}
		}
		
		
		/*! Marshal the inputs of the HMM for the given node.
		 */
		__GPU_DECL__ int get_inputs(hmm_header* hdr, node_header* ndr) {
			int x=0;
			for(int i=0; i<ndr->nin; ++i) {
				int ni=ndr->xinput(i);
				x |= (hdr->tminus1(ni) & 0x01) << (ndr->nin-1-i);
			}
			return x;
		}	
		
		
		/*! Calculate the output of a probabilistic node.
		 */
		__GPU_DECL__ void probabilistic_output(hmm_header* hdr, node_header* ndr, int rnum) {
			int rowidx=get_inputs(hdr,ndr);
			table_header* tdr=reinterpret_cast<table_header*>(ndr);
			
			int* row = tdr->row(rowidx);
			rnum %= row[tdr->ncol-1];
			int col=0;
			while(rnum > row[col]) {
				rnum -= row[col];
				++col;
			}
			
			set_outputs(col,hdr,ndr);
		}
		
		
		/*! Calculate the output of a deterministic node.
		 */
		__GPU_DECL__ void deterministic_output(hmm_header* hdr, node_header* ndr) {
			int rowidx=get_inputs(hdr,ndr);
			table_header* tdr=reinterpret_cast<table_header*>(ndr);
			set_outputs(tdr->entry(rowidx,0), hdr, ndr);
		}
		
	} // hmm
} // fn

#endif
