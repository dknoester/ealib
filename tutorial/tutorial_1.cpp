/* tutorial_1.cpp
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
#include "tutorial_1.h"

int main(int argc, const char * argv[]) {
    repr_type r1(repr_size,1);
    cout << "10 ones: " << all_ones(r1) << endl;
    
    repr_type r2(repr_size,0);
    cout << "0 ones: " << all_ones(r2) << endl;
    return 0;
}

/* Great.  However, there's no evolution here -- Indeed, we don't even have a
 population.  On to part 2.
 */
