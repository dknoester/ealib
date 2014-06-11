/* test_types.cpp
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
#include <boost/test/unit_test.hpp>

#include <ea/analysis.h>
#include <ea/cmdline_interface.h>
#include <ea/digital_evolution.h>
#include <ea/subpopulation_founder.h>
#include <ea/line_of_descent.h>
#include <ea/metapopulation.h>
#include "test.h"

//! Configuration object; this is fairly standard across DE simulations.

struct digevo_lifecycle : public default_lifecycle {
    //! Called as the final step of EA construction (must not depend on configuration parameters).
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
        typedef typename EA::resource_ptr_type resource_ptr_type;
        
        // Add tasks
        task_ptr_type task_not = make_task<tasks::task_not,catalysts::additive<0> >("not", ea);
        task_ptr_type task_nand = make_task<tasks::task_nand,catalysts::additive<0> >("nand", ea);
        task_ptr_type task_and = make_task<tasks::task_and,catalysts::additive<0> >("and", ea);
        task_ptr_type task_ornot = make_task<tasks::task_ornot,catalysts::additive<0> >("ornot", ea);
        task_ptr_type task_or = make_task<tasks::task_or,catalysts::additive<0> >("or", ea);
        task_ptr_type task_andnot = make_task<tasks::task_andnot,catalysts::additive<0> >("andnot", ea);
        task_ptr_type task_nor = make_task<tasks::task_nor,catalysts::additive<0> >("nor", ea);
        task_ptr_type task_xor = make_task<tasks::task_xor,catalysts::additive<0> >("xor", ea);
        task_ptr_type task_equals = make_task<tasks::task_equals,catalysts::additive<0> >("equals", ea);
        
        resource_ptr_type resA = make_resource("resA", 100.0, 1.0, 0.01, 0.05, ea);
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


//! Meta-population w/ founders configuration object.
struct mp_founder_lifecycle : public default_lifecycle {
    template <typename EA>
    void initial_population(EA& ea) {
        for(typename EA::iterator i=ea.begin(); i!=ea.end(); ++i) {
            (*i).founder() = (**(*i).population().begin());
        }
    }
};

LIBEA_ANALYSIS_TOOL(test_population_lod_tool) {
    using namespace ealib;
//    using namespace ealib::analysis;
    
    line_of_descent<EA> lod = lod_load(get<ANALYSIS_INPUT>(ea), ea);
    typename line_of_descent<EA>::iterator i=lod.begin(); ++i;
    
    for( ; i!=lod.end(); ++i) {
        typename EA::individual_ptr_type control_ea = ea.make_individual();
        typename EA::individual_type::individual_ptr_type o = i->make_individual(i->founder().repr());
        o->hw().initialize();
        control_ea->append(o);
    }
}


template <typename EA>
class cli : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
    }
    
    virtual void gather_tools() {
        add_tool<test_population_lod_tool>(this);
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::mrca_lineage>(this,ea);
        add_event<subpopulation_founder_event>(this,ea);
    };
};


//! A variety of digital evolution / artificial life simulation definitions.
BOOST_AUTO_TEST_CASE(test_digevo_types) {
    //! Single population:
    typedef digital_evolution<digevo_lifecycle > ea_type1;
    ea_type1 ea1;
    
    //! Meta-population, no founders:
    typedef metapopulation<ea_type1> mea_type1;
    mea_type1 mea1;
    
    //    //! Meta-population, with founders:
    //    typedef meta_population<population_founder<ea_type1> > mea_type2;
    //    mea_type2 mea2;
    //
    //    //! Meta-population, with founders and LOD tracking:
    //    typedef meta_population<
    //    population_founder<ea_type1>,
    //    ancestors::default_representation,
    //    mutation::operators::no_mutation,
    //	constant,
    //    abstract_configuration,
    //	recombination::no_recombination,
    //    generational_models::isolated_subpopulations,
    //    attr::lod_attributes
    //    > mea_type3;
    //    cli<mea_type3> cli3;
}


