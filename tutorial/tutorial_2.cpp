/* tutorial_2.cpp
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

// Go here:
#include "tutorial_2.h"

int main(int argc, const char * argv[]) {
    population_type p(population_size, repr_type(repr_size, 0));
    
    random_selection(p);
    return 0;
}

/* Great.  However, there's still no evolution here -- There's no mutation or
 preferential survival for highly fit individuals.  On to part 3...
 */
