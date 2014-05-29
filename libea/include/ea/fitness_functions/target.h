/* target.h
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
#ifndef _EA_FITNESS_FUNCTIONS_TARGET_H_
#define _EA_FITNESS_FUNCTIONS_TARGET_H_

#include <algorithm>
#include <functional>
#include <ea/fitness_function.h>

namespace ealib {
	
	/*! Fitness function that rewards for the number of codons matching a given value.
	 
     (Primarily for testing.)
	 */
    template <typename T>
	struct target : public fitness_function<unary_fitness<double> > {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            return static_cast<double>(std::count_if(ind.repr().begin(), ind.repr().end(),
                                                     std::bind2nd(std::equal_to<T>(), target)));
		}
        
        T target;
	};
}

#endif
