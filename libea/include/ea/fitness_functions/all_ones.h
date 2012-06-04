#ifndef _EA_FITNESS_FUNCTIONS_ALL_ONES_H_
#define _EA_FITNESS_FUNCTIONS_ALL_ONES_H_

#include <algorithm>
#include <ea/fitness_function.h>

namespace ea {
	
	/*! Fitness function that rewards for the number of ones in the genome.
	 
     (Primarily for testing.)
	 */
	struct all_ones : public fitness_function<unary_fitness<double> > {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
			return static_cast<double>(std::count(ind.repr().begin(), ind.repr().end(), 1u));
		}
	};

}

#endif
