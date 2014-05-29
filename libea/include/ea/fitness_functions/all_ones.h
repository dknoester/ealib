/* all_ones.h
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
#ifndef _EA_FITNESS_FUNCTIONS_ALL_ONES_H_
#define _EA_FITNESS_FUNCTIONS_ALL_ONES_H_

#include <algorithm>
#include <ea/fitness_function.h>

namespace ealib {
	
	/*! Fitness function that rewards for the number of ones in the genome.
	 
     (Primarily for testing.)
	 */
	struct all_ones : public fitness_function<unary_fitness<double> > {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
			return static_cast<double>(std::count(ind.genome().begin(), ind.genome().end(), 1u));
		}
	};

    struct multi_all_ones : public fitness_function<multivalued_fitness<double> > {
        double range(std::size_t m) {
            return 1.0;
        }
        
		template <typename Individual, typename EA>
        value_type operator()(Individual& ind, EA& ea) {
            value_type f;
            std::copy(ind.repr().begin(), ind.repr().end(), std::back_inserter(f));
            return f;
		}
	};

}

#endif
