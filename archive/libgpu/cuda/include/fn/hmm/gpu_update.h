/* gpu_update.h
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
#ifndef _FN_HMM_GPU_UPDATE_H_
#define _FN_HMM_GPU_UPDATE_H_

namespace fn {
	namespace hmm {
		
		struct hmm_header;
		
		//! Allocate memory for, and copy an HMM network to, a GPU.
		void* gpu_alloc(hmm_header* hdr, std::size_t size);
		
		//! Deallocate memory on a GPU.
		void gpu_free(void* dmem);
		
		//! Update this network on the GPU.
		void gpu_update(hmm_header* hdr, void* dmem, int seed);
		
	} // hmm
} // fn

#endif
