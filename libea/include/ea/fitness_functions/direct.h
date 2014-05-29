/* direct.h
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
#ifndef _EA_FITNESS_FUNCTIONS_DIRECT_H_
#define _EA_FITNESS_FUNCTIONS_DIRECT_H_

#include <algorithm>

#include <ea/fitness_functions/ff_base.h>

namespace ealib {
	
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
