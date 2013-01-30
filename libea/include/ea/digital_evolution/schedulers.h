/* digital_evolution/digital_evolution.h 
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
#include <map>
#include <ea/generational_model.h>
#include <ea/fitness_function.h>

namespace ea {    
    
    LIBEA_MD_DECL(SCHEDULER_TIME_SLICE, "ea.scheduler.time_slice", unsigned int);
    
    /*! Weighted round-robin scheduler.
     
     Grants all organisms an amount of CPU time proportional to their priority.
     
     priority == multiple of cycles above an org that has priority 1.0.
     
     Fractional updates: There is a fractional update that occurs time-slice times
     per update (once every population-size cpu cycles).
     */
    template <typename EA>
    struct weighted_round_robin : generational_models::generational_model {
        typedef EA ea_type;
        typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
        
        void initialize(ea_type& ea) {
        }
               
        template <typename Population>
        void operator()(Population& population, ea_type& ea) {
            typedef std::vector<std::size_t> exc_list;
            exc_list live;
            int last=population.size();
            for(std::size_t i=0; i<population.size(); ++i) {
                int r=static_cast<int>(population[i]->priority());
                for(int j=0; j<r; ++j) {
                    live.push_back(i);
                }
            }
            
            std::random_shuffle(live.begin(), live.end(), ea.rng());
            
            unsigned int eff_population_size = std::min(static_cast<unsigned int>(population.size()),get<POPULATION_SIZE>(ea));
            long budget=get<SCHEDULER_TIME_SLICE>(ea) * eff_population_size;
            double delta_t = 1.0/get<SCHEDULER_TIME_SLICE>(ea);
            
            std::size_t i=0;
            int deadcount=0;
            while((budget > 0) && (deadcount<last)) {
                if((budget % eff_population_size) == 0) {
                    ea.env().partial_update(delta_t, ea);
                }
                
                typename ea_type::individual_ptr_type p=population[live[i]];
                i = (i+1) % live.size();

                if(p->alive()) {
                    p->execute(1,p,ea);
                    --budget;
                } else {
                    ++deadcount;
                }
            }
            
            Population next;
            for(std::size_t i=0; i<population.size(); ++i) {
                typename ea_type::individual_ptr_type p=population[i];
                if(p->alive()) {
                    next.push_back(p);
                }
            }
            std::swap(population, next);
        }
    };
    
    
    /*! Round-robin scheduler.
     
     Grants all organisms an equal amount of CPU time, exactly time slice cycles
     per update.
     */
    template <typename EA>
    struct round_robin : generational_models::generational_model {
        typedef EA ea_type;
        typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
        
        void initialize(ea_type& ea) {
        }
        
        template <typename Population>
        void operator()(Population& population, ea_type& ea) {
            // WARNING: Population is unstable!  Must use []-indexing.
            std::random_shuffle(population.begin(), population.end(), ea.rng());

            // these are the individuals in the population at the start of the update.
            // they are the *only* ones that can execute during this update,
            // and some of them are likely to be replaced.
            // offspring are appended to population asynchronously, thus we're
            // indexing into the population instead of iterating.
            
            unsigned int eff_population_size = std::min(static_cast<unsigned int>(population.size()),get<POPULATION_SIZE>(ea));
            long budget=get<SCHEDULER_TIME_SLICE>(ea) * eff_population_size;
            double delta_t = 1.0/get<SCHEDULER_TIME_SLICE>(ea);
            
            std::size_t last=population.size();
            std::size_t i=0;
            std::size_t deadcount=0;
            while((budget > 0) && (deadcount<last)) {
                if((budget % eff_population_size) == 0) {
                    ea.env().partial_update(delta_t, ea);
                }
                typename ea_type::individual_ptr_type p=population[i];
                i = (i+1) % last;
                
                if(p->alive()) {
                    p->execute(1,p,ea);
                    --budget;
                } else {
                    ++deadcount;
                }
            }
            
            Population next;
            for(std::size_t i=0; i<population.size(); ++i) {
                typename ea_type::individual_ptr_type p=population[i];
                if(p->alive()) {
                    next.push_back(p);
                }
            }
            std::swap(population, next);
        }
    };
    
} // ea

#endif
