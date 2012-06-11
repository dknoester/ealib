/* artificial_life/artificial_life.h 
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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

#ifndef _EA_SCHEDULERS_H_
#define _EA_SCHEDULERS_H_

#include <limits>
#include <ea/generational_model.h>
#include <ea/fitness_function.h>

namespace ea {    
    
    // in all cases, random_walk the population?  check the sgi stl extension "random_sample"
    
    // fitness_proportional; fitness_i/sum(priorities) cycles
    LIBEA_MD_DECL(SCHEDULER_TIME_SLICE, "ea.scheduler.time_slice", unsigned int);

    struct round_robin : generational_models::generational_model {
        typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
        
        template <typename AL>
        void initialize(AL& al) {
        }
        
        template <typename Population, typename AL>
        void operator()(Population& population, AL& al) {
            // WARNING! The population is *unstable*.  it must be indexed (or, while loop w/ pop_front?):
            
            unsigned int t=get<SCHEDULER_TIME_SLICE>(al);
            std::random_shuffle(population.begin(), population.end(), al.rng());
            Population next;
            
            for(std::size_t i=0; i<population.size(); ++i) {
                typename AL::individual_ptr_type p=ptr(population[i],al);                
                if(p->alive()) {
                    // just born?  don't execute.
                    if(p->update() != current_update()) {
                        p->execute(t,p,al);
                    }
                    // all organisms that are alive get pushed into the next generation.
                    // note that they could still die before they get scheduled next!
                    next.append(population[i]);
                }
            }
            std::swap(next, population);
        }
    };
    
} // ea

#endif
