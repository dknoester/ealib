/* tutorial_4.cpp
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
#include "tutorial_4.h"

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
        random_selection_with_smart_mutation(p);
    }
    
    return 0;
}

/* Zounds!  The output should now look something like this:
 
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
 
 A significant improvement.  In fact, the maximum fitness was reached after only
 a single update.
 
 But, from a software perspective, what have we done?  A whole lot of copy & paste.
 Why is this bad?  Go to part 5 to see why, and fix some of it.
 */
