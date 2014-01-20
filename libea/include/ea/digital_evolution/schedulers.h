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

#include <boost/bind.hpp>
#include <boost/signal.hpp>
#include <limits>
#include <list>
#include <map>
#include <ea/fitness_function.h>

namespace ealib {    
    
    LIBEA_MD_DECL(SCHEDULER_TIME_SLICE, "ea.scheduler.time_slice", unsigned int);
    
    typedef unary_fitness<double> priority_type; //!< Type for storing priorities.

    namespace traits {

        template <typename T>
        struct priority_trait {
            //! Returns this individual's priority.
            priority_type& priority() { return _v; }
            
            template <class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                ar & boost::serialization::make_nvp("priority", _v);
            }
            
            priority_type _v;
        };
        
    } // traits
    
    namespace access {
        
        //! Priority accessor functor.
        struct priority {
            template <typename EA>
            priority_type& operator()(typename EA::individual_type& ind, EA& ea) {
                return ind.traits().priority();
            }
        };
        
    } // access

    //! Priority attribute accessor.
    template <typename EA>
    priority_type& priority(typename EA::individual_type& ind, EA& ea) {
        return ind.priority();
    }
    
    /*! Runlevel queue scheduler.
     
     Here, individuals are sorted into different runlevels.  Each runlevel receives
     twice as many virtual cpu cycles as the runlevel immediately below it.  All individuals
     in the current runlevel are granted a single virtual cpu cycle before the runlevel
     is advanced.  This avoids strange runlevel size effects (i.e., where a higher runlevel
     has twice as many individuals as a lower runlevel).
     */
    template <typename EA>
    struct runlevel_queue {
        typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
        typedef std::list<typename EA::individual_ptr_type> queue_type;
        typedef std::map<int,queue_type> runlevelq_type;
        
        void initialize(EA& ea) {
        }
        
        template <typename Population>
        void operator()(Population& population, EA& ea) {
            // shuffle the population to avoid effects related to ordering of individuals:
            std::random_shuffle(population.begin(), population.end(), ea.rng());

            // split them into runlevels:
            runlevelq_type q;
            for(typename Population::iterator i=population.begin(); i!=population.end(); ++i) {
                // we have to take the ceiling of the priority here; otherwise, an organism
                // that consumes only a small amount is effectively the same as an organism
                // that consumes none; this creates a relavitely large subset of the population
                // that can drift while the rest of the population competes for resources.
                q[ceil(priority(**i,ea))].push_back(*i);
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
                    ea.env().partial_update(delta_t, ea);
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
            
            Population next;
            next.reserve(population.size());
            for(std::size_t i=0; i<population.size(); ++i) {
                if(population[i]->alive()) {
                    next.push_back(population[i]);
                }
            }
            std::swap(population, next);
        }
    };
    

    /*! Priority-proportional scheduler.
     
     Grants all organisms an amount of CPU time proportional to their priority,
     where priority is defined as the multiple of cycles above an org that has 
     priority 1.0.
     */
    template <typename EA>
    struct priority_proportional {
        typedef EA ea_type;
        typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
        typedef std::vector<long> exc_list;
        
        void initialize(ea_type& ea) {
        }
        
        template <typename Population>
        exc_list operator()(Population& population, ea_type& ea) {
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
                    ea.env().partial_update(delta_t, ea);
                }
                
                typename ea_type::individual_ptr_type p=population[live[i]];
                i = (i+1) % live.size();
                names.push_back(p->name());
                
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
            return names;
        }
    };

    
    /*! Weighted round-robin scheduler.
     
     Grants organisms an amount of CPU time equal to their priority.
     */
    struct weighted_round_robin {
        typedef unary_fitness<double> priority_type; //!< Type for storing priorities.

        template <typename Population, typename EA>
        void operator()(Population& population, EA& ea) {
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
                typename EA::individual_ptr_type p=population[i];
                i = (i+1) % last;
                
                if(p->alive()) {
                    p->execute(static_cast<std::size_t>(ealib::priority(*p,ea)),p,ea);
                    budget -= static_cast<std::size_t>(ealib::priority(*p,ea));
                } else {
                    ++deadcount;
                }
            }
            
            Population next;
            for(std::size_t i=0; i<population.size(); ++i) {
                typename EA::individual_ptr_type p=population[i];
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
    struct round_robin {
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
    
} // ealib

//
//    /*! Priority tree-based scheduler.
//     */
//    template <typename EA>
//    struct priority_tree : generational_models::generational_model {
//        typedef EA ea_type;
//        typedef unary_fitness<double> priority_type; //!< Type for storing priorities.
//
//        void initialize(EA& ea) {
////            _birth_conn = ea.events().birth.connect(boost::bind(&priority_tree::on_birth, this, _1, _2, _3));
////            _death_conn = ea.events().death.connect(boost::bind(&priority_tree::on_death, this, _1, _2));
//        }
//
////        void on_birth(typename EA::individual_type& offspring, typename EA::individual_type& parent, EA& ea) {
////        }
////
////        void on_death(typename EA::individual_type& ind, EA& ea) {
////        }
//
//
////        boost::signals::scoped_connection _birth_conn, _death_conn;
//    };

#endif
