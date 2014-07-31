/* digital_evolution/schedulers.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2014 David B. Knoester, Heather J. Goldsby.
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

#include <list>
#include <map>
#include <ea/fitness_function.h>

namespace ealib {    
    
    LIBEA_MD_DECL(SCHEDULER_TIME_SLICE, "ea.scheduler.time_slice", unsigned int);
    LIBEA_MD_DECL(SCHEDULER_RESOURCE_SLICE, "ea.scheduler.resource_slice", unsigned int);
    
    typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
    
    namespace access {
        
        //! Priority accessor functor.
        struct priority {
            template <typename EA>
            priority_type& operator()(typename EA::individual_type& ind, EA& ea) {
                return ind.priority();
            }
        };

        //! Accessor that retuns the unit priority (1.0) for all individuals.
        struct unit_priority {
            template <typename EA>
            double operator()(typename EA::individual_type& ind, EA& ea) {
                return 1.0;
            }
        };

    } // access


    /*! Weighted round-robin scheduler.
     
     Executes all individuals in a round-robin fashion, granting each individual
     a number CPU cycles equal to their priority during each execution.

     This scheduler operates in O(N + M) time, where N is population size and M
     is the number of virtual CPU cycles scheduled during an update.
     */
    template <typename PriorityAccessor=access::priority>
    struct weighted_round_robin {
        typedef PriorityAccessor accessor_type;

        template <typename EA>
        void operator()(typename EA::population_type& population, EA& ea) {
            // only the individuals in the population at the start of this update
            // are allowed to execute, and some of them are likely to be replaced.
            // offspring are appended to population **asynchronously.**
            //
            // WARNING: Population is unstable!  Must use []-indexing.
            std::random_shuffle(population.begin(), population.end(), ea.rng());

            const unsigned int eff_population_size = std::min(static_cast<unsigned int>(population.size()),get<POPULATION_SIZE>(ea));
            const long budget=get<SCHEDULER_TIME_SLICE>(ea) * eff_population_size;
            const double delta_t = 1.0/static_cast<double>(get<SCHEDULER_RESOURCE_SLICE>(ea));
            const long ncycles_per_period = budget * delta_t;
            const std::size_t N=population.size();

            long consumed=0; // total consumed CPU cycles
            int last_period=-1; // update period
            std::size_t i=0; // current index into population vector
            std::size_t deadcount=0;
            
            while((consumed < budget) && (deadcount < N)) {
                // updates are divided into periods, where each period corresponds
                // to a partial resource update:
                int period=consumed/ncycles_per_period;
                if(period != last_period) {
                    ea.resources().update(delta_t);
                    last_period = period;
                }
                
                typename EA::individual_ptr_type p=population[i];
                if(p->alive()) {
                    std::size_t n=static_cast<std::size_t>(_acc(*p,ea));
                    p->execute(n, p, ea);
                    consumed += n;
                } else {
                    ++deadcount;
                }
                
                // new individuals are appended to the population; don't execute
                // them during this update:
                i = (i+1) % N;
            }
            
            // prune all dead organisms from the population:
            typename EA::population_type next;
            next.reserve(get<POPULATION_SIZE>(ea));
            for(std::size_t i=0; i<population.size(); ++i) {
                typename EA::individual_ptr_type p=population[i];
                if(p->alive()) {
                    next.push_back(p);
                }
            }
            std::swap(population, next);
        }
        
        //! Link a standing population to this scheduler.
        template <typename EA>
        void link(EA& ea) {
        }
        
        accessor_type _acc; //!< Accessor for an individual's priority.
    };
    
    /*! Round-robin scheduler.
     
     Executes all organisms in a round-robin fashion, granting each a single
     CPU instruction per execution.
     */
    typedef weighted_round_robin<access::unit_priority> round_robin;

} // ealib

#endif
