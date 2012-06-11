/* truncation.h
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
#ifndef _EA_SELECTION_TRUNCATION_H_
#define _EA_SELECTION_TRUNCATION_H_

#include <algorithm>
#include <iterator>
#include <ea/algorithm.h>
#include <ea/comparators.h>

namespace ea {
	namespace selection {
		
		/*! Truncation selection.
		 
		 This selection method truncates a population by removing low-fitness
         individuals.

		 <b>Model of:</b> SelectionStrategyConcept.
		 */
		struct truncation {
            //! Initializing constructor.
			template <typename Population, typename EA>
			truncation(std::size_t n, Population& src, EA& ea) {
            }

			//! Copy the n most-fit individuals to dst.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                assert(src.size() >= n);
                std::sort(src.begin(), src.end(), comparators::fitness());
                typename Population::reverse_iterator rl=src.rbegin();
                std::advance(rl, n);
                dst.append(src.rbegin(), rl);
			}
		};

	} // selection
} // ealib

#endif
