/* tournament.h
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
#ifndef _EA_SELECTION_TOURNAMENT_H_
#define _EA_SELECTION_TOURNAMENT_H_

#include <algorithm>
#include <iterator>
#include <ea/algorithm.h>
#include <ea/comparators.h>
#include <ea/meta_data.h>

namespace ea {
	namespace selection {
		
		/*! Tournament selection.
		 
		 This selection method runs tournaments of size N and selects the K individuals
		 with greatest fitness.
		 
		 <b>Model of:</b> SelectionStrategyConcept.
		 */
        template <typename Comparator=comparators::fitness>
		struct tournament {
            //! Initializing constructor.
			template <typename Population, typename EA>
			tournament(std::size_t n, Population& src, EA& ea) { 
            }

			//! Select n individuals via tournament selection.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
				std::size_t N = get<TOURNAMENT_SELECTION_N>(ea);
				std::size_t K = get<TOURNAMENT_SELECTION_K>(ea);
				while(n > 0) {
					Population tourney;
					std::insert_iterator<Population> tii(tourney,tourney.end());
					ea.rng().sample_with_replacement(src.begin(), src.end(), tii, N);
					
                    std::sort(tourney.begin(), tourney.end(), Comparator());
                    typename Population::reverse_iterator rl=tourney.rbegin();
                    std::size_t copy_size = std::min(n,K);
                    std::advance(rl, copy_size);
                    dst.insert(dst.end(), tourney.rbegin(), rl);
					n -= copy_size;
				}
			}
		};
        
	} // selection
} // ealib

#endif
