/* quiet_nan.h
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
#ifndef _EA_FITNESS_FUNCTIONS_QUIET_NAN_H_
#define _EA_FITNESS_FUNCTIONS_QUIET_NAN_H_

#include <limits>
#include <ea/fitness_function.h>

namespace ealib {
	
    //! Fitness function that assigns NaN to all individuals.
	struct quiet_nan : public fitness_function<unary_fitness<double> > {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            std::numeric_limits<double>::quiet_NaN();
		}
	};
    
}

#endif
