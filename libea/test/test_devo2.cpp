/* test_ealife.cpp
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
#include "test.h"
#include <ea/digital_evolution.h>

using namespace ealib;

struct test_configuration : public default_configuration {
    
    //! Called as the final step of EA construction.
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
        append_isa<input>(ea);
        append_isa<fixed_input>(ea);
        append_isa<output>(ea);
        append_isa<repro>(ea);
    }
    
    template <typename EA>
    void initialize(EA& ea) {
        typedef typename EA::task_library_type::task_ptr_type task_ptr_type;
        typedef typename EA::environment_type::resource_ptr_type resource_ptr_type;
        
        task_ptr_type task_nand = make_task<tasks::task_nand,catalysts::additive<1> >("nand", ea);
        resource_ptr_type resA = make_resource("resA", ea);
        task_nand->consumes(resA);
    }
    
};

typedef digital_evolution
< organism< >
, selfrep_ancestor
, recombination::asexual
, weighted_round_robin
, random_neighbor
, dont_stop
, test_configuration
> ea_type;


/*!
 */
BOOST_AUTO_TEST_CASE(test_devo2) {
    ea_type ea;
    put<POPULATION_SIZE>(100,ea);
	put<REPRESENTATION_SIZE>(100,ea);
	put<MUTATION_PER_SITE_P>(0.0075,ea);
    put<SCHEDULER_TIME_SLICE>(30,ea);
    put<SPATIAL_X>(10,ea);
    put<SPATIAL_Y>(10,ea);

    lifecycle::prepare_new(ea);
    lifecycle::advance_epoch(10,ea);
}
