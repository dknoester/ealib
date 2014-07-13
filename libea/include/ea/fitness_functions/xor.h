/* pole_balancing.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
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
#ifndef _EA_FITNESS_FUNCTIONS_XOR_H_
#define _EA_FITNESS_FUNCTIONS_XOR_H_

#include <boost/math/constants/constants.hpp>

#include <ea/fitness_function.h>
#include <ea/metadata.h>

namespace ealib {
	
    LIBEA_MD_DECL(XOR_TRIALS, "ea.fitness_function.xor.trials", int);
    
	/*! Fitness function for the 2-input XOR problem.
     */
	struct two_input_xor : public fitness_function<unary_fitness<double>, constantS, stochasticS> {
        /*! Evaluate the fitness of a single individual.
         */
        template <typename Individual, typename RNG, typename EA>
		double operator()(Individual& ind, RNG& rng, EA& ea) {
            double input[3]; // inputs to the phenotype
            typename EA::phenotype_type &P = ealib::phenotype(ind, ea); // phenotype; ANN, MKV, etc.
            int trials = get<XOR_TRIALS>(ea);
            double f=0.0;
            for(int i=0; i<trials; ++i) {
                input[0] = 1.0; // bias
                input[1] = rng.bit();
                input[2] = rng.bit();
                
                P.update(input, input+3);
                f += (*P.begin_output() > 0.5) == (static_cast<int>(input[1]) ^ static_cast<int>(input[2]));
            }
            
            return f;
        }
	};
    
} // ealib

#endif
