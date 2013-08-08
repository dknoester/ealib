/* elitism.h
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
#ifndef _EA_SELECTION_ELITISM_H_
#define _EA_SELECTION_ELITISM_H_

#include <algorithm>
#include <iterator>
#include <ea/algorithm.h>
#include <ea/comparators.h>
#include <ea/meta_data.h>

namespace ealib {
	namespace selection {
		
		/*! Stacks elitism on top of another selection strategy.
		 
		 This selection strategy "stacks" with others; that is, it must be used in
		 conjunction with another selection strategy, such as tournament_selection.
		 Elitism augments that selection strategy by explicitly preserving N "elite"
		 (high-fitness) individuals.  Those selected are *still* maintained as part
		 of the source population from which the embedded selection strategy draws its
		 own selected individuals.
		 */
 		template <typename SelectionStrategy>
		struct elitism {
			typedef SelectionStrategy embedded_selection_type;

			//! Initializing constructor.
			template <typename Population, typename EA>
			elitism(std::size_t n, Population& src, EA& ea) : _embedded(n,src,ea) {
			}
            
			/*! Preserve the elite individuals from the src population.
             */
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
				std::size_t e = get<ELITISM_N>(ea);
                assert(n > e);
                _embedded(src, dst, n-e, ea);
                
                // now, append the e most-fit individuals:
                if(e > 0) {
                    std::sort(src.begin(), src.end(), comparators::fitness<EA>(ea));
                    typename Population::reverse_iterator rl=src.rbegin();
                    std::advance(rl, e);
                    dst.insert(dst.end(), src.rbegin(), rl);
                }
            };

			embedded_selection_type _embedded; //!< Underlying selection strategy.
		};

	} // selection
} // ealib

#endif
