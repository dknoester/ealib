/* digital_evolution/utils/resource_consumption.h
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

/* Tracks resource consumption for logic 9 tasks. Also can track at subpop level.*/


#ifndef _EA_DIGITAL_EVOLUTION_UTILS_RESOURCE_CONSUMPTION_H_
#define _EA_DIGITAL_EVOLUTION_UTILS_RESOURCE_CONSUMPTION_H_

#include <ea/digital_evolution.h>
#include <ea/digital_evolution/hardware.h>
#include <ea/digital_evolution/instruction_set.h>
#include <ea/digital_evolution/discrete_spatial_environment.h>
#include <ea/datafiles/reactions.h>
#include <ea/cmdline_interface.h>
#include <ea/metapopulation.h>
#include <ea/selection/random.h>
#include <ea/mutation.h>

using namespace ealib;


LIBEA_MD_DECL(GROUP_RESOURCE_UNITS, "ea.res.group_resource_units", double);
LIBEA_MD_DECL(SAVED_RESOURCES, "ea.res.organism_saved_resources", double);
LIBEA_MD_DECL(GROUP_REP_THRESHOLD, "ea.res.group_rep_threshold", double);

LIBEA_MD_DECL(TASK_NOT, "ea.not", double);
LIBEA_MD_DECL(TASK_NAND, "ea.nand", double);
LIBEA_MD_DECL(TASK_AND, "ea.and", double);
LIBEA_MD_DECL(TASK_ORNOT, "ea.ornot", double);
LIBEA_MD_DECL(TASK_OR, "ea.or", double);
LIBEA_MD_DECL(TASK_ANDNOT, "ea.andnot", double);
LIBEA_MD_DECL(TASK_NOR, "ea.nor", double);
LIBEA_MD_DECL(TASK_XOR, "ea.xor", double);
LIBEA_MD_DECL(TASK_EQUALS, "ea.equals", double);

/*! Donate an organism's resources to the group. 
 */

DIGEVO_INSTRUCTION_DECL(donate_res_to_group){
    get<GROUP_RESOURCE_UNITS>(ea, 0.0) += get<SAVED_RESOURCES>(*p,0.0);
    put<SAVED_RESOURCES>(0,*p);
}



/*! Tracks an organism's resources and tasks. 
 */

template <typename EA>
struct task_resource_consumption : reaction_event<EA> {
    task_resource_consumption(EA& ea) : reaction_event <EA>(ea) {
    }
    
    virtual ~task_resource_consumption() { }
    virtual void operator()(typename EA::individual_type& ind, // individual
                            typename EA::task_library_type::task_ptr_type task, // task pointer   
                            double r,
                            EA& ea) {
        get<SAVED_RESOURCES>(ind, 0.0) += r;
        std::string t = task->name();
        if (t == "not") { get<TASK_NOT>(ea,0.0) += 1.0; get<TASK_NOT>(ind,0.0) += 1.0; }
        else if (t == "nand") { get<TASK_NAND>(ea,0.0) += 1.0; get<TASK_NAND>(ind,0.0) += 1.0; }
        else if (t == "and") { get<TASK_AND>(ea,0.0) += 1.0; get<TASK_AND>(ind,0.0) += 1.0; }
        else if (t == "ornot") { get<TASK_ORNOT>(ea,0.0) += 1.0; get<TASK_ORNOT>(ind,0.0) += 1.0; }
        else if (t == "or") { get<TASK_OR>(ea,0.0) += 1.0; get<TASK_OR>(ind,0.0) += 1.0; }
        else if (t == "andnot") { get<TASK_ANDNOT>(ea,0.0) += 1.0; get<TASK_ANDNOT>(ind,0.0) += 1.0; }
        else if (t == "nor") { get<TASK_NOR>(ea,0.0) += 1.0; get<TASK_NOR>(ind,0.0) += 1.0; }
        else if (t == "xor") { get<TASK_XOR>(ea,0.0) += 1.0; get<TASK_XOR>(ind,0.0) += 1.0; }
        else if (t == "equals") { get<TASK_EQUALS>(ea,0.0) += 1.0; get<TASK_EQUALS>(ind,0.0) += 1.0; }
        
    }
};

/*! Prints information about the aggregate task performance of the group.
 */


template <typename EA>
struct task_performed_tracking : end_of_update_event<EA> {
    task_performed_tracking(EA& ea) : end_of_update_event<EA>(ea), _df("tasks.dat") { 
        _df.add_field("update")
        .add_field("not")
        .add_field("nand")
        .add_field("and")
        .add_field("ornot")
        .add_field("or")
        .add_field("andnot")
        .add_field("nor")
        .add_field("xor")
        .add_field("equals");
    }
    
    //! Destructor.
    virtual ~task_performed_tracking() {
    }
    
    //! Track resources!
    virtual void operator()(EA& ea) {
        if ((ea.current_update() % 100) == 0) {
            int t_not = 0;
            int t_nand = 0;
            int t_and = 0;
            int t_ornot = 0; 
            int t_or = 0;
            int t_andnot = 0;
            int t_nor = 0;
            int t_xor = 0;
            int t_equals = 0;
            
            for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
                t_not += get<TASK_NOT>(*i, 0.0);
                t_nand += get<TASK_NAND>(*i, 0.0);
                t_and += get<TASK_AND>(*i, 0.0);
                t_ornot += get<TASK_ORNOT>(*i, 0.0);
                t_or += get<TASK_OR>(*i, 0.0);
                t_andnot += get<TASK_ANDNOT>(*i, 0.0);
                t_nor += get<TASK_NOR>(*i, 0.0);
                t_xor += get<TASK_XOR>(*i, 0.0);
                t_equals += get<TASK_EQUALS>(*i, 0.0);
            }
            
            _df.write(ea.current_update())
            .write(t_not)
            .write(t_nand)
            .write(t_and)
            .write(t_ornot)
            .write(t_or)
            .write(t_andnot)
            .write(t_nor)
            .write(t_xor)
            .write(t_equals)
            .endl();
        }
        
    }
    datafile _df;    
    
};


#endif
