/* logic9.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
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
using namespace ealib;

/*! Configures an instance of digital evolution in a manner similar to Avida.
 
 This configuration is for a fairly typical Avida-like system for the evolution
 of the EQU logic task (bitwise equals).
 */
struct lifecycle : public default_lifecycle {
    //! Called after EA initialization.
    template <typename EA>
    void after_initialization(EA& ea) {
        // for compatibility with emscripten, we're fully qualifying the names of
        // some instructions.  for c++ only, feel free to drop them.
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
        append_isa<ealib::instructions::swap>(ea); // namespace clash with std; emscripten bug
        append_isa<inc>(ea);
        append_isa<ealib::instructions::dec>(ea); // ""
        append_isa<tx_msg>(ea);
        append_isa<rx_msg>(ea);
        append_isa<bc_msg>(ea);
        append_isa<ealib::instructions::rotate>(ea); // ""
        append_isa<rotate_cw>(ea);
        append_isa<rotate_ccw>(ea);
        append_isa<if_less>(ea);
        append_isa<h_alloc>(ea);
        append_isa<h_copy>(ea);
        append_isa<h_divide>(ea);
        append_isa<fixed_input>(ea);
        append_isa<output>(ea);

        typedef typename EA::task_library_type::task_ptr_type task_ptr_type;
        typedef typename EA::resource_ptr_type resource_ptr_type;
        
        // tasks:
        task_ptr_type task_not = make_task<tasks::task_not,catalysts::additive<1> >("not", ea);
        task_ptr_type task_nand = make_task<tasks::task_nand,catalysts::additive<1> >("nand", ea);
        task_ptr_type task_and = make_task<tasks::task_and,catalysts::additive<2> >("and", ea);
        task_ptr_type task_ornot = make_task<tasks::task_ornot,catalysts::additive<2> >("ornot", ea);
        task_ptr_type task_or = make_task<tasks::task_or,catalysts::additive<2> >("or", ea);
        task_ptr_type task_andnot = make_task<tasks::task_andnot,catalysts::additive<3> >("andnot", ea);
        task_ptr_type task_nor = make_task<tasks::task_nor,catalysts::additive<3> >("nor", ea);
        task_ptr_type task_xor = make_task<tasks::task_xor,catalysts::additive<3> >("xor", ea);
        task_ptr_type task_equals = make_task<tasks::task_equals,catalysts::additive<4> >("equals", ea);
        
        // resources:
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
typedef digital_evolution<lifecycle> ea_type;
