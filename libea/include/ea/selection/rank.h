/* rank.h
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
#ifndef _EA_SELECTION_RANK_H_
#define _EA_SELECTION_RANK_H_

#include <algorithm>
#include <iterator>
#include <ea/comparators.h>

namespace ealib {
    
	namespace selection {
		
		/*! Selects individuals based on the rank of their fitness.
		 */
		struct rank {
			//! Constructor.
			template <typename Population, typename EA>
			rank(std::size_t n, Population& src, EA& ea) {
			}
            
			//! Select n individuals via rank selection.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                std::sort(src.begin(), src.end(), comparators::fitness<EA>(ea));
                std::copy_n(src.rbegin(), std::min(src.size(),n), std::inserter(dst,dst.end()));
            }
        };

	} // selection
} // ealib

#endif
