#include <cuda.h>

#include <fn/hmm/gpu_update.h>
#include <fn/hmm/layout.h>
#include <fn/hmm/output.h>


/*! Allocate memory for, and copy an HMM network to, a GPU.
 */
void* fn::hmm::gpu_alloc(hmm_header* hdr, std::size_t size) {
	void* dmem=0;
	cudaMalloc(&dmem, size);
	assert(dmem);
	cudaMemcpy(dmem, hdr, size, cudaMemcpyHostToDevice);
	return dmem;
}


/*! Deallocate memory on a GPU.
 */
void fn::hmm::gpu_free(void* dmem) {
	assert(dmem);
	cudaFree(dmem);
}


/*! Generate a park-miller psuedorandom number.
 
 Derived from Langdon, GECCO'09.
 */
__device__ int parkmiller_rand(unsigned int seed, int cycles) {
	float const a=16807;
	float const m=2147483647;
	float const reciprocal_m = 1.0/m;
	unsigned int data=seed+blockDim.x*blockIdx.x + threadIdx.x;
	unsigned int result=0;
	
	for(int i=1; i<=cycles; ++i) {
		float temp = data * a;
		result = (int) (temp - m * floor(temp * reciprocal_m));
		data = result;
	}
	
	return result;
}


/*! Generate a psuedorandom number via linear congruential generation.
 */
__device__ int lcg_rand(int a) {
	return ((long)a * 279470273UL) % 4294967291UL;
}


/*! Update the GPU's copy of the HMM.
 */
__global__ void update(void* dmem, int seed) {
	using namespace fn::hmm;
	hmm_header* hdr=reinterpret_cast<hmm_header*>(dmem);
	node_header* ndr=reinterpret_cast<node_header*>(hdr->node_ptr(threadIdx.x));
	
	switch(ndr->start_codon[0]) {
		case 42: { // probabilistic node
			probabilistic_output(hdr,ndr,parkmiller_rand(seed,1));
			break;
		}
		case 43: { // deterministic node
			deterministic_output(hdr,ndr);
			break;
		}
		default:
			break;
	}
}


/*! Update this network on the GPU.
 
 In this case, we're copying the whole header to the device before the update,
 running the update, and then copying the header back to the host.  We can't get
 away with just copying the state vectors, as we need the offsets for t and t-1.
 
 So, in order to reduce the number of memcpys, we copy a few extra bytes and grab
 the whole header.
 */		
void fn::hmm::gpu_update(hmm_header* hdr, void* dmem, int seed) {
	// copy the header to the device:
	cudaMemcpy(dmem, hdr, hdr->header_size(), cudaMemcpyHostToDevice);
	
	update<<<1,hdr->nnodes>>>(dmem, seed);
	
	// copy the header from the device:
	cudaMemcpy(hdr, dmem, hdr->header_size(), cudaMemcpyDeviceToHost);
}
