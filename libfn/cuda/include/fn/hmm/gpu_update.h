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
