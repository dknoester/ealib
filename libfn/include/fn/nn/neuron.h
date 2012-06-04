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
