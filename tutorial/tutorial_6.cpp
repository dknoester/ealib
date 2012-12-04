/* tutorial_6.cpp
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
#include "tutorial_6.h"

/* Now wrap everything up in a loop, and run it.
 */
int main(int argc, const char * argv[]) {
    // for grins, let's bump up the repr_size:
    repr_size = 100;
    population_size = 1000;
    
    population_type p(population_size, repr_type(repr_size, 0));
    
    // each time through this loop is an "update":
    for(std::size_t i=0; i<10; ++i) {
        // let's first print out the mean & max fitness each time through:
        double sum=0.0;
        double max_fitness=0.0;
        for(population_type::iterator j=p.begin(); j!=p.end(); ++j) {
            double f = all_ones(*j);
            sum += f;
            max_fitness = std::max(max_fitness, f);
        }
        std::cout << "mean: " << sum/p.size() << " max: " << max_fitness << std::endl;
        
        // and now the EA:
        compete(p);

        generic_random_selection(p, flip);
//        generic_random_selection(p, random_bit);
    }
    
    return 0;
}

/* Ok, the output should still look something like this:
 
 mean: 0 max: 0
 mean: 4.55 max: 100
 mean: 79.407 max: 100
 mean: 94.674 max: 100
 mean: 94.288 max: 100
 mean: 94.416 max: 100
 mean: 94.305 max: 100
 mean: 94.33 max: 100
 mean: 94.694 max: 100
 mean: 94.323 max: 100
 
 Great!  Now we can remove all the non-generic versions of mutate and random_selection,
 reducing the amount of code to maintain, while increasing it's reusability.
 
 So, what have we done here?  We started with a fairly simple EA that contained all
 the major parts of an EA.  We then refined it by removing duplicated code, and
 added two generic functions for mutation and selection.
 
 EALib continues this process, and provides generic components that allow all
 major parts of an EA to be varied.
 */
