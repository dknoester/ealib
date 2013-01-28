/* neuron.h
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
#ifndef _NN_NEURON_H_
#define _NN_NEURON_H_

namespace nn {

	/*! Base neuron type.
	 */
	struct neuron_base {
		//! Possible neuron types.
		enum neuron_type { INPUT, HIDDEN, OUTPUT, INACTIVE };
		
		//! Constructor.
		neuron_base(neuron_type t) : _type(t) { }
		
		neuron_type _type; //!< Type of this neuron.
	};

} // nn

#endif
