#ifndef _EA_FITNESS_FUNCTIONS_DIRECT_H_
#define _EA_FITNESS_FUNCTIONS_DIRECT_H_

#include <algorithm>

#include <ea/fitness_functions/ff_base.h>

namespace ea {
	
	/*! Fitness function that simply accumulates the value of each loci.
     
     (Primarily for testing.)
	 */
	struct direct : public fitness_function<double> {
		template <typename Representation, typename EA>
		double operator()(Representation& rep, EA& ea) {
			return (double)std::accumulate(rep.begin(), rep.end(), 0.0);
		}
	};
	
} // ea

#endif
