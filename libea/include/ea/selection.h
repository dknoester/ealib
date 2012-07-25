/* selection.h
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
#ifndef _EA_SELECTION_H_
#define _EA_SELECTION_H_

#include <ea/algorithm.h>
#include <ea/interface.h>
#include <ea/meta_data.h>


namespace ea {

    /*! Select n individuals from src into dst using the given selector type.
     
     This is "survivor selection" -- The near-final step of most generational models,
     immediately prior to population swaps (if any).  As such, this is where relative 
     fitness is calculated, if the fitness function specifies it.
	 */
	template <typename Selector, typename Population, typename EA>
	void select_n(Population& src, Population& dst, std::size_t n, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
        ea.preselect(src);
        Selector select(n,src,ea);
        select(src, dst, n, ea);
	}
    

} // ea

#endif
