/* tutorial_3.cpp
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
#include "tutorial_3.h"

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
        random_selection_with_mutation(p);
    }
    
    return 0;
}

/* Ok, good.  We now have a very simple, but fully functional EA.  The output 
 should look something like this:

 mean: 0 max: 0
 mean: 2.264 max: 58
 mean: 36.231 max: 58
 mean: 48.844 max: 60
 mean: 49.425 max: 60
 mean: 49.801 max: 62
 mean: 50.141 max: 62
 mean: 50.538 max: 64
 mean: 51.11 max: 67
 mean: 51.456 max: 67
 
 Hm.  Not bad, but not great.  We'll start improving it in part 4.
 */
