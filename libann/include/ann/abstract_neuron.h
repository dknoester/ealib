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
#ifndef _ANN_ABSTRACT_NEURON_H_
#define _ANN_ABSTRACT_NEURON_H_

namespace ann {
    
    namespace neuron {
        //! Flag bits for neurons.
        enum neuron_flags { reserved=0x01, top=0x02, bias=0x04, input=0x08, output=0x010, hidden=0x020 };
    }
    
    typedef unsigned int neuron_flags;
    
	/*! Abstract base class for neurons.
	 */
	struct abstract_neuron {
        
		//! Constructor.
		abstract_neuron() : input(0.0), output(0.0), _flags(0) { }
        
        bool getf(neuron_flags f) { return _flags & f; }
        void setf(neuron_flags f) { _flags |= f; }
        neuron_flags flags() { return _flags; }
        neuron_flags flags(neuron_flags f) { _flags = f; return _flags; }
        
        neuron_flags _flags; //!< Flags set for this neuron.
        double input; //!< Input to this neuron.
		double output; //!< Output from this neuron.
	};
    
} // nn

#endif
