/* digital_evolution.cpp
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

#include <ea/digital_evolution.h>
#include <ea/cmdline_interface.h>
using namespace ealib;

/*! Configures an instance of digital evolution in a manner similar to Avida.
 
 This configuration is for a fairly typical Avida-like system for the evolution
 of the EQU logic task (bitwise equals).
 */
struct lifecycle : public default_lifecycle {
    
    //! Called as the final step of EA construction (must not depend on configuration parameters)
    template <typename EA>
    void after_construction(EA& ea) {
        using namespace ealib::instructions;
        append_isa<nop_a>(0,ea);
        append_isa<nop_b>(0,ea);
        append_isa<nop_c>(0,ea);
        append_isa<nop_x>(ea);
        append_isa<mov_head>(ea);
        append_isa<if_label>(ea);
        append_isa<h_search>(ea);
        append_isa<nand>(ea);
        append_isa<push>(ea);
        append_isa<pop>(ea);
        append_isa<swap>(ea);
        append_isa<inc>(ea);
        append_isa<dec>(ea);
        append_isa<tx_msg>(ea);
        append_isa<rx_msg>(ea);
        append_isa<bc_msg>(ea);
        append_isa<rotate>(ea);
        append_isa<rotate_cw>(ea);
        append_isa<rotate_ccw>(ea);
        append_isa<if_less>(ea);
        append_isa<h_alloc>(ea);
        append_isa<h_copy>(ea);
        append_isa<h_divide>(ea);
        append_isa<fixed_input>(ea);
        append_isa<output>(ea);
    }
    
    //! Initialize the EA (may use configuration parameters)
    template <typename EA>
    void initialize(EA& ea) {
        typedef typename EA::task_library_type::task_ptr_type task_ptr_type;
        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type;

        // Add tasks
        task_ptr_type task_not = make_task<tasks::task_not,catalysts::additive<1> >("not", ea);
        task_ptr_type task_nand = make_task<tasks::task_nand,catalysts::additive<1> >("nand", ea);
        task_ptr_type task_and = make_task<tasks::task_and,catalysts::additive<2> >("and", ea);
        task_ptr_type task_ornot = make_task<tasks::task_ornot,catalysts::additive<2> >("ornot", ea);
        task_ptr_type task_or = make_task<tasks::task_or,catalysts::additive<2> >("or", ea);
        task_ptr_type task_andnot = make_task<tasks::task_andnot,catalysts::additive<3> >("andnot", ea);
        task_ptr_type task_nor = make_task<tasks::task_nor,catalysts::additive<3> >("nor", ea);
        task_ptr_type task_xor = make_task<tasks::task_xor,catalysts::additive<3> >("xor", ea);
        task_ptr_type task_equals = make_task<tasks::task_equals,catalysts::additive<4> >("equals", ea);
        
        
        resource_ptr_type resA = make_resource("resA", 0.1, 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resB = make_resource("resB", 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resC = make_resource("resC", 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resD = make_resource("resD", 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resE = make_resource("resE", 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resF = make_resource("resF", 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resG = make_resource("resG", 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resH = make_resource("resH", 100.0, 1.0, 0.01, 0.05, ea);
        resource_ptr_type resI = make_resource("resI", 100.0, 1.0, 0.01, 0.05, ea);
        
        task_not->consumes(resA);
        task_nand->consumes(resB);
        task_and->consumes(resC);
        task_ornot->consumes(resD);
        task_or->consumes(resE);
        task_andnot->consumes(resF);
        task_nor->consumes(resG);
        task_xor->consumes(resH);
        task_equals->consumes(resI);
    }
};


/*! Artificial life simulation definition.
 */
typedef digital_evolution<
lifecycle
> ea_type;
    
    
/*!
 */
template <typename EA>
class cli : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
        add_option<SPATIAL_X>(this);
        add_option<SPATIAL_Y>(this);
        add_option<POPULATION_SIZE>(this);
        add_option<REPRESENTATION_SIZE>(this);
        add_option<SCHEDULER_TIME_SLICE>(this);
        add_option<MUTATION_PER_SITE_P>(this);
        add_option<MUTATION_INSERTION_P>(this);
        add_option<MUTATION_DELETION_P>(this);
        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<CHECKPOINT_PREFIX>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);
    }
    
    virtual void gather_tools() {
    }
    
    virtual void gather_events(EA& ea) {
    };
};

LIBEA_CMDLINE_INSTANCE(ea_type, cli);