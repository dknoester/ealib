/* gpu_network.cpp
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
