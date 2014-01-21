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

#include <ea/fitness_function.h>

namespace ealib {

    /*! Select at most n individuals from src into dst using the given selector type.
     This is "survivor selection" -- The near-final step of most generational models,
     immediately prior to population swaps (if any).
	 */
	template <typename Selector, typename Population, typename EA>
	void select_n(Population& src, Population& dst, std::size_t n, EA& ea) {
        if(src.size() <= n) {
            dst.insert(dst.end(), src.begin(), src.end());
        } else {
            Selector select(n,src,ea);
            select(src, dst, n, ea);
        }
	}    

} // ea

#endif
