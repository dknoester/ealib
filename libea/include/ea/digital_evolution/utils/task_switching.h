/* digital_evolution/utils/task_switching.h
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


#ifndef _EA_DIGITAL_EVOLUTION_UTILS_TASK_SWITCHING_H_
#define _EA_DIGITAL_EVOLUTION_UTILS_TASK_SWITCHING_H_



#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/algorithm/string.hpp>


#include <ea/digital_evolution.h>
#include <ea/digital_evolution/hardware.h>
#include <ea/digital_evolution/instruction_set.h>
#include <ea/digital_evolution/environment.h>
#include <ea/digital_evolution/utils/resource_consumption.h>
#include <ea/digital_evolution/utils/configurable_mutation.h>
#include <ea/datafiles/reactions.h>
#include <ea/cmdline_interface.h>
#include <ea/metapopulation.h>
#include <ea/selection/random.h>
#include <ea/mutation.h>

using namespace ealib;
using namespace boost::accumulators;

LIBEA_MD_DECL(TASK_SWITCHING_COST, "ea.ts.task_switching_cost", int);
LIBEA_MD_DECL(LAST_TASK, "ea.ts.last_task", std::string);
LIBEA_MD_DECL(NUM_SWITCHES, "ea.ts.num_switches", int);
LIBEA_MD_DECL(GERM_MUTATION_PER_SITE_P, "ea.ts.germ_mutation_per_site_p", double);
LIBEA_MD_DECL(NUM_GROUP_REPLICATIONS, "ea.ts.num_group_replications", int);
LIBEA_MD_DECL(TASK_PROFILE, "ea.ts.task_profile", std::string);


LIBEA_MD_DECL(RES_INITIAL_AMOUNT, "ea.ts.res_initial_amount", double);
LIBEA_MD_DECL(RES_INFLOW_AMOUNT, "ea.ts.res_inflow_amount", double);
LIBEA_MD_DECL(RES_OUTFLOW_FRACTION, "ea.ts.res_outflow_fraction", double);
LIBEA_MD_DECL(RES_FRACTION_CONSUMED, "ea.ts.res_fraction_consumed", double);




/*! If an organism changes tasks, then it incurs a task-switching cost.
 */

template <typename EA>
struct task_switching_cost : reaction_event<EA> {
    
    task_switching_cost(EA& ea) : reaction_event<EA>(ea) {
    }
    
    virtual ~task_switching_cost() { }
    virtual void operator()(typename EA::individual_type& ind, // individual
                            typename EA::task_library_type::task_ptr_type task, // task pointer
                            double r, // amount of resource consumed
                            EA& ea) {
        
        if (exists<LAST_TASK>(ind) && 
            (task->name() != get<LAST_TASK>(ind, ""))) {
            
            ind.hw().add_cost(get<TASK_SWITCHING_COST>(ea)); 
            get<NUM_SWITCHES>(ind, 0) += 1; 
        }
        put<LAST_TASK>(task->name(), ind); 
        
    }
};



/*! Track an organism's task profile.
 */

template <typename EA>
struct task_profile_tracking : reaction_event<EA> {
    
    task_profile_tracking(EA& ea) : reaction_event<EA>(ea) {
    }
    
    virtual ~task_profile_tracking() { }
    virtual void operator()(typename EA::individual_type& ind, // individual
                            typename EA::task_library_type::task_ptr_type task, // task pointer
                            double r, // amount of resource consumed
                            EA& ea) {
        
        // task->name()
        std::string t = task->name();
        if (t == "not") { get<TASK_PROFILE>(ind,"") += "0"; }
        else if (t == "nand") { get<TASK_PROFILE>(ind,"") += "1"; }
        else if (t == "and") { get<TASK_PROFILE>(ind,"") += "2"; }
        else if (t == "ornot") { get<TASK_PROFILE>(ind,"") += "3"; }
        else if (t == "or") { get<TASK_PROFILE>(ind,"") += "4"; }
        else if (t == "andnot") { get<TASK_PROFILE>(ind,"") += "5";  }
        else if (t == "nor") { get<TASK_PROFILE>(ind,"") += "6"; }
        else if (t == "xor") { get<TASK_PROFILE>(ind,"") += "7"; }
        else if (t == "equals") { get<TASK_PROFILE>(ind,"") += "8"; }
    }
};



/*! Prints information about the mean number of task-switches
 */


template <typename EA>
struct task_switch_tracking : end_of_update_event<EA> {
    task_switch_tracking(EA& ea) : end_of_update_event<EA>(ea), _df("ts.dat") { 
        _df.add_field("update")
        .add_field("sub_pop_size")
        .add_field("pop_size")
        .add_field("mean_ts")
        .add_field("mc_rep");
        
    }
    
    //! Destructor.
    virtual ~task_switch_tracking() {
    }
    
    //! Track how many task-switches are being performed!
    virtual void operator()(EA& ea) {
        if ((ea.current_update() % 100) == 0) {
            double ts = 0;
            double org = 0;
            
            int sub_pop_size = 0;
            
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                ++sub_pop_size;
                for(typename EA::subpopulation_type::iterator j=i->begin(); j!=i->end(); ++j){
                    typename EA::subpopulation_type::individual_type& ind=*j;
                    if (ind.alive()) {
                        ts += get<NUM_SWITCHES>(ind, 0);
                        ++org;
                    }
                }
            }
            ts /= org;
            _df.write(ea.current_update())
            .write(sub_pop_size)
            .write(org)
            .write(ts)
            .write(get<NUM_GROUP_REPLICATIONS>(ea,0))
            .endl();
            
            get<NUM_GROUP_REPLICATIONS>(ea) = 0;
        }
        
    }
    datafile _df;    
    
};



//! Performs group replication using germ lines.
template <typename EA>
struct ts_replication : end_of_update_event<EA> {
    //! Constructor.
    ts_replication(EA& ea) : end_of_update_event<EA>(ea) {
    }
    
    
    //! Destructor.
    virtual ~ts_replication() {
    }
    
    //! Perform germline replication among populations.
    virtual void operator()(EA& ea) {
        
        // See if any subpops have exceeded the resource threshold
        typename EA::population_type offspring;
        for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
            
            // Do not replicate if the 'founding org' is sterile.
            if (i->population().size() < 2) continue; 
            
            if (exists<GROUP_RESOURCE_UNITS>(*i) && 
                (get<GROUP_RESOURCE_UNITS>(*i) > get<GROUP_REP_THRESHOLD>(*i))){
                
                // grab a copy of the founder!
                
                typename EA::individual_type::individual_type prop = (*i).founder();
                prop.repr().resize((*i).founder().hw().original_size());

                prop.hw().initialize();
                
                
                // setup the population (really, an ea):
                typename EA::individual_ptr_type p = ea.make_individual();
                
                // mutate it:
                configurable_per_site m(get<GERM_MUTATION_PER_SITE_P>(ea)); 
                mutate(prop,m,*p);
                
                // and fill up the offspring population with copies of the germ:
                typename EA::individual_type::individual_ptr_type o=p->make_individual(prop.repr());
                p->append(o);
                offspring.push_back(p);
                
                // reset resource units
                i->env().reset_resources();
                put<GROUP_RESOURCE_UNITS>(0,*i);
                
                // i == parent individual;
                typename EA::population_type parent_pop, offspring_pop;
                parent_pop.push_back(*i.base());
                offspring_pop.push_back(p);
                inherits(parent_pop, offspring_pop, ea);
            }
        }
        
        get<NUM_GROUP_REPLICATIONS>(ea,0) += offspring.size();
        
        // select surviving parent groups
        if (offspring.size() > 0) {
            int n = get<METAPOPULATION_SIZE>(ea) - offspring.size(); 
            
            typename EA::population_type survivors;
            select_n<selection::random>(ea.population(), survivors, n, ea);
            
            // add the offspring to the list of survivors:
            survivors.insert(survivors.end(), offspring.begin(), offspring.end());
            
            // and swap 'em in for the current population:
            std::swap(ea.population(), survivors);
        }
       
    }

    
    
};
 



/*! An organism rotates to face its parent....
 */
template <typename EA>
struct ts_birth_event : birth_event<EA> {
    
    //! Constructor.
    ts_birth_event(EA& ea) : birth_event<EA>(ea) {
    }
    
    //! Destructor.
    virtual ~ts_birth_event() {
    }
    
    /*! Called for every inheritance event. We are using the orientation of the first parent...
     */
    virtual void operator()(typename EA::individual_type& offspring, // individual offspring
                            typename EA::individual_type& parent, // individual parent
                            EA& ea) {
        ea.env().face_org(parent, offspring);
        
    }
};



/*! An organism rotates to face its parent....
 */
template <typename EA>
struct task_profile_birth_event : birth_event<EA> {
    
    //! Constructor.
    task_profile_birth_event(EA& ea) : birth_event<EA>(ea) {
    }
    
    //! Destructor.
    virtual ~task_profile_birth_event() {
    }
    
    /*! Called for every inheritance event. We are using the orientation of the first parent...
     */
    virtual void operator()(typename EA::individual_type& offspring, // individual offspring
                            typename EA::individual_type& parent, // individual parent
                            EA& ea) {
         get<TASK_PROFILE>(parent,"") += "+";
        
    }
};



#endif



