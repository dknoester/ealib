#include <fn/hmm/gpu_network.h>

/*! Constructor.
 */
fn::hmm::gpu_network::gpu_network(const genome& g, std::size_t in, std::size_t out, std::size_t hidden)
: hmm_network(g,in,out,hidden), _device_mem(0) {
	_device_mem = gpu_alloc(_h, _mem_needed);
}


/*! Destructor.
 */
fn::hmm::gpu_network::~gpu_network() {
	if(_device_mem) {
		gpu_free(_device_mem);
	}
}
