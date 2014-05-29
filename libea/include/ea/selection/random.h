/* random.h
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
#ifndef _EA_SELECTION_RANDOM_H_
#define _EA_SELECTION_RANDOM_H_

#include <iterator>
#include <ea/selection.h>

namespace ealib {	
	namespace selection {
		
		/*! Random selection.

		 <b>Model of:</b> SelectionStrategyConcept.
		 */
        template <typename ReplacementTag=without_replacementS>
		struct random {
            typedef ReplacementTag replacement_tag;
            
            //! Initializing constructor.
			template <typename Population, typename EA>
			random(std::size_t n, Population& src, EA& ea) {
			}
            
            //! Select n individuals at random.
			template <typename Population, typename EA>
			void select(Population& src, Population& dst, std::size_t n, EA& ea, with_replacementS) {
				ea.rng().sample_with_replacement(src.begin(), src.end(), std::inserter(dst, dst.end()), n);
            }
            
			//! Select n individuals at random.
			template <typename Population, typename EA>
			void select(Population& src, Population& dst, std::size_t n, EA& ea, without_replacementS) {
				ea.rng().sample_without_replacement(src.begin(), src.end(), std::inserter(dst, dst.end()), n);
            }
                
			//! Select n individuals at random.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                select(src, dst, n, ea, replacement_tag());
			}
		};
        
	} // selection
} // ealib

#endif
