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
    
    typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
    
    namespace access {
        
        //! Priority accessor functor.
        struct priority {
            template <typename EA>
            priority_type& operator()(typename EA::individual_type& ind, EA& ea) {
                return ind.priority();
            }
        };
        
    } // access

    
    /*! Runlevel queue scheduler.
     
     Here, individuals are sorted into different runlevels.  Each runlevel receives
     twice as many virtual cpu cycles as the runlevel immediately below it.  All individuals
     in the current runlevel are granted a single virtual cpu cycle before the runlevel
     is advanced.  This avoids strange runlevel size effects (i.e., where a higher runlevel
     has twice as many individuals as a lower runlevel).
     */
    struct runlevel_queue {
        //! Schedule organisms.
        template <typename EA>
        void operator()(typename EA::population_type& population, EA& ea) {
            typedef std::list<typename EA::individual_ptr_type> queue_type;
            typedef std::map<int,queue_type> runlevelq_type;

            // shuffle the population to avoid effects related to ordering of individuals:
            std::random_shuffle(population.begin(), population.end(), ea.rng());

            // split them into runlevels:
            runlevelq_type q;
            for(typename EA::population_type::iterator i=population.begin(); i!=population.end(); ++i) {
                // we have to take the ceiling of the priority here; otherwise, an organism
                // that consumes only a small amount is effectively the same as an organism
                // that consumes none; this creates a relavitely large subset of the population
                // that can drift while the rest of the population competes for resources.
                q[ceil((*i)->priority())].push_back(*i);
            }
            
            // have to fill in the runlevels between priorities, if any:
            // not sure about this, actually.  the problem is that scheduling slows **way**
            // down if the priority distribution is sparse.
            for(int i=1; i<q.rbegin()->first; ++i) {
                q[i]; // "touch"; constructs if needed.
            }
            
            unsigned int eff_population_size = std::min(static_cast<unsigned int>(population.size()),get<POPULATION_SIZE>(ea));
            long budget=get<SCHEDULER_TIME_SLICE>(ea) * eff_population_size;
            double delta_t = 1.0/get<SCHEDULER_TIME_SLICE>(ea);
            std::size_t livecount=population.size();
            std::size_t deadcount=0;
            typename runlevelq_type::reverse_iterator runlevel=q.rbegin(), last=q.rbegin();
            typename queue_type::iterator i=runlevel->second.begin();
            
            while((budget > 0) && (deadcount<livecount)) {
                if((budget % eff_population_size) == 0) {
                    ea.resources().update(delta_t);
                }
                
                if(i != runlevel->second.end()) {
                    typename EA::individual_ptr_type p=*i;
                    if(p->alive()) {
                        p->execute(1,p,ea);
                        --budget;
                        ++i;
                    } else {
                        runlevel->second.erase(i++);
                        ++deadcount;
                    }
                } else {
                    if(runlevel == last) {
                        runlevel = q.rbegin();
                        ++last;
                    } else {
                        ++runlevel;
                    }
                    
                    if(last == q.rend()) {
                        last = q.rbegin();
                    }

                    i = runlevel->second.begin();
                }
            }
            
            typename EA::population_type next;
            next.reserve(population.size());
            for(std::size_t i=0; i<population.size(); ++i) {
                if(population[i]->alive()) {
                    next.push_back(population[i]);
                }
            }
            std::swap(population, next);
        }
        
        //! Link a standing population to this scheduler.
        template <typename EA>
        void link(EA& ea) {
        }
    };
    

    /*! Priority-proportional scheduler.
     
     Grants all organisms an amount of CPU time proportional to their priority,
     where priority is defined as the multiple of cycles above an org that has 
     priority 1.0.
     */
    struct priority_proportional {
        //! Schedule organisms.
        template <typename EA>
        void operator()(typename EA::population_type& population, EA& ea) {
            typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
            typedef std::vector<long> exc_list;
            

            exc_list live, names;
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
                    ea.resources().update(delta_t);
                }
                
                typename EA::individual_ptr_type p=population[live[i]];
                i = (i+1) % live.size();
                names.push_back(p->name());
                
                if(p->alive()) {
                    p->execute(1,p,ea);
                    --budget;
                } else {
                    ++deadcount;
                }
            }
            
            typename EA::population_type next;
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
    };

    
    /*! Weighted round-robin scheduler.
     
     Grants organisms an amount of CPU time equal to their priority.
     */
    struct weighted_round_robin {
        template <typename EA>
        void operator()(typename EA::population_type& population, EA& ea) {
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
                    ea.resources().update(delta_t);
                }
                typename EA::individual_ptr_type p=population[i];
                i = (i+1) % last;
                
                if(p->alive()) {
                    p->execute(static_cast<std::size_t>(p->priority()),p,ea);
                    budget -= static_cast<std::size_t>(p->priority());
                } else {
                    ++deadcount;
                }
            }
            
            typename EA::population_type next;
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
    };
    
    
    /*! Round-robin scheduler.
     
     Grants all organisms an equal amount of CPU time, exactly time slice cycles
     per update.
     */
    struct round_robin {
        //! Schedule organisms.
        template <typename EA>
        void operator()(typename EA::population_type& population, EA& ea) {
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
                    ea.resources().update(delta_t);
                }
                typename EA::individual_ptr_type p=population[i];
                i = (i+1) % last;
                
                if(p->alive()) {
                    p->execute(1,p,ea);
                    --budget;
                } else {
                    ++deadcount;
                }
            }
            
            typename EA::population_type next;
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
    };
    
    /* Faster scheduler; will need to templatify this, and likely turn it into
     the default scheduler.
     
     class ProbSchedule
     {
     private:
     const int num_items;
     std::vector<double> weights;
     std::vector<double> tree_weights;
     Random m_rng;
     
     ProbSchedule(const ProbSchedule&); // @not_implemented
     ProbSchedule& operator=(const ProbSchedule&); // @not_implemented
     
     int CalcID(double rand_pos, int cur_id) {
     // If our target is in the current node, return it!
     const double cur_weight = weights[cur_id];
     if (rand_pos < cur_weight) return cur_id;
     
     // Otherwise determine if we need to recurse left or right.
     rand_pos -= cur_weight;
     const int left_id = cur_id*2 + 1;
     const double left_weight = tree_weights[left_id];
     
     return (rand_pos < left_weight) ? CalcID(rand_pos, left_id) : CalcID(rand_pos-left_weight, left_id+1);
     }
     
     public:
     ProbSchedule(int _items, int seed=-1) : num_items(_items), weights(_items+1), tree_weights(_items+1), m_rng(seed) {
     for (int i = 0; i < (int) weights.size(); i++)  weights[i] = tree_weights[i] = 0.0;
     }
     ~ProbSchedule() { ; }
     
     double GetWeight(int id) const { return weights[id]; }
     double GetSubtreeWeight(int id) const { return tree_weights[id]; }
     
     void Adjust(int id, const double in_weight) {
     weights[id] = in_weight;
     
     // Determine the child ids to adjust subtree weight.
     const int left_id = 2*id + 1;
     const int right_id = 2*id + 2;
     
     // Make sure the subtrees looked for haven't fallen off the end of this tree.
     const double st1_weight = (left_id < num_items) ? tree_weights[left_id] : 0.0;
     const double st2_weight = (right_id < num_items) ? tree_weights[right_id] : 0.0;
     tree_weights[id] = in_weight + st1_weight + st2_weight;
     
     // Cascade the change up the tree to the root.
     while (id) {
     id = (id-1) / 2;
     tree_weights[id] = weights[id] + tree_weights[id*2+1] + tree_weights[id*2+2];
     }
     }
     
     int NextID() {
     const double total_weight = tree_weights[0];
     
     // Make sure it's possible to schedule...
     if (total_weight == 0.0) return -1;
     
     // If so, choose a random number to use for the scheduling.
     double rand_pos = m_rng.GetDouble(total_weight);
     return CalcID(rand_pos, 0);
     }
     };

     */
    
} // ealib

#endif
