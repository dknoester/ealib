/* test_digital_evolution.cpp
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
#include "test.h"
#include <ea/digital_evolution.h>


struct test_lifecycle : default_lifecycle {
    //! Called after EA initialization.
    template <typename EA>
    void after_initialization(EA& ea) {
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

        typedef typename EA::task_library_type::task_ptr_type task_ptr_type;
        typedef typename EA::resource_ptr_type resource_ptr_type;
        
        task_ptr_type task_nand = make_task<tasks::task_nand,catalysts::additive<1> >("nand", ea);
        resource_ptr_type resA = make_resource("resA", ea);
        task_nand->consumes(resA);
    }
};

typedef digital_evolution
< test_lifecycle
> ea_type;

metadata build_md() {
    metadata md;
    put<POPULATION_SIZE>(100,md);
	put<REPRESENTATION_SIZE>(100,md);
    put<SPATIAL_X>(10,md);
    put<SPATIAL_Y>(10,md);
    put<SCHEDULER_TIME_SLICE>(30,md);
    put<SCHEDULER_RESOURCE_SLICE>(30,md);
    put<MUTATION_PER_SITE_P>(0.0075,md);
    put<CHECKPOINT_PREFIX>("checkpoint",md);
    put<RNG_SEED>(1,md);
    put<RECORDING_PERIOD>(10,md);
    return md;
}

BOOST_AUTO_TEST_CASE(test_resources) {
    ea_type ea(build_md());
    ea_type::resource_ptr_type r = make_resource("resB", 0.1, 0.5, 1.0, 0.75, 0.1, ea);
    
    for(std::size_t k=0; k<20; ++k) {
        //        std::cout << k << std::endl;
        //        for(std::size_t y=get<SPATIAL_Y>(ea); y>0; --y) {
        //            for(std::size_t i=0; i<get<SPATIAL_X>(ea); ++i) {
        //                std::cout << r->level(make_position(i,y-1)) << " ";
        //            }
        //            std::cout << std::endl;
        //        }
        r->update(1);
    }
    
    BOOST_CHECK_CLOSE(0.128744, r->level(position_type(0,0)), 0.001);
    BOOST_CHECK_CLOSE(0.0721839, r->level(position_type(1,0)), 0.001);
}

BOOST_AUTO_TEST_CASE(test_avida_hardware) {
    ea_type ea(build_md());
    ea_type::isa_type& isa=ea.isa();
    generate_ancestors(nopx_ancestor(), 1, ea);
    
    ea_type::individual_ptr_type p = ea.population()[0];
    ea_type::genome_type& r = p->repr();
    ea_type::hardware_type& hw = p->hw();
    
    r[8] = isa["nop_a"];
    r[9] = isa["nop_b"];
    r[10] = isa["nop_c"];
    
    // Check setting and getting of register values
    hw.setRegValue(hardware::AX, 27);
	BOOST_CHECK_EQUAL(hw.getRegValue(hardware::AX), 27);
    
    // Check setting and getting of head locations
    hw.setHeadLocation(hardware::IP, 5);
	BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::IP), 5);
    
    // Advance a head location - should deal with circular genome
    hw.advanceHead(hardware::FH, 102);
	BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::FH), 2);
    
    // Find a label's complement
    hw.pushLabelStack(hardware::NOP_C);
    hw.pushLabelStack(hardware::NOP_A);
    hw.pushLabelStack(hardware::NOP_B);
    std::deque<int> comp_label = hw.getLabelComplement();
    BOOST_CHECK_EQUAL(comp_label[0], 0);
    BOOST_CHECK_EQUAL(comp_label[1], 1);
    BOOST_CHECK_EQUAL(comp_label[2], 2);
    
    std::pair<int, int> c = hw.findComplementLabel();
    BOOST_CHECK_EQUAL(c.first, 3);
    BOOST_CHECK_EQUAL(c.second, 3);
}

BOOST_AUTO_TEST_CASE(test_avida_instructions) {
    ea_type ea(build_md());
    ea_type::isa_type& isa=ea.isa();
    generate_ancestors(nopx_ancestor(), 1, ea);
    
    ea_type::individual_ptr_type p = ea.population()[0];
    ea_type::genome_type& r = p->repr();
    ea_type::hardware_type& hw = p->hw();
    
    r[4] = isa["nop_a"];
    r[5] = isa["nop_b"];
    r[6] = isa["nop_c"];
    
    // Check if mov-head does in fact move the IP head to the position
    // before the FH (counting on the advance mechanism to put them
    // in the same place
    // FH at 10
    hw.setHeadLocation(hardware::FH, 10);
    isa(isa["mov_head"], p->hw(), p, ea);
	BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::IP), 9);
    
    // Check h-search when complement is found
    // Create our label.
    hw.pushLabelStack(hardware::NOP_C);
    hw.pushLabelStack(hardware::NOP_A);
    hw.pushLabelStack(hardware::NOP_B);
    isa(isa["h_search"], p->hw(), p, ea);
    
    // If a complement is found, BX is set to the distance to the complement,
    BOOST_CHECK_EQUAL(hw.getRegValue(hardware::BX), 95);
    
    // CX is set to its size,
    BOOST_CHECK_EQUAL(hw.getRegValue(hardware::CX), 3);
    
    // and the flow head is set to the instruction immediately
    // following the complement.
    BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::FH), 7);
}


BOOST_AUTO_TEST_CASE(test_self_replicator_instructions) {
    ea_type ea(build_md());
    ea_type::isa_type& isa=ea.isa();
    generate_ancestors(nopx_ancestor(), 1, ea);
    
    ea_type::individual_ptr_type p = ea.population()[0];
    ea_type::genome_type& r = p->repr();
    
    // check that h_alloc increases the size of the organism's memory:
    isa(isa["h_alloc"], p->hw(), p, ea);
    BOOST_CHECK_EQUAL(r.size(), 250u);
    
    // Check that h_copy:
    // (1) Copies the instruction at the read head to the location pointed to by
    // the write head
    r[10] = isa["input"];
    p->hw().setHeadLocation(hardware::RH, 10);
    p->hw().setHeadLocation(hardware::WH, 20);
    
    isa(isa["h_copy"], p->hw(), p, ea);
	BOOST_CHECK_EQUAL(r[20], isa["input"]);
    
    // (2) Advances the read and write head positions by 1.
	BOOST_CHECK_EQUAL(p->hw().getHeadLocation(hardware::RH), 11);
	BOOST_CHECK_EQUAL(p->hw().getHeadLocation(hardware::WH), 21);
}

BOOST_AUTO_TEST_CASE(test_logic9_environment) {
    // test input/output
    ealib::tasks::task_not tnot;
    ealib::tasks::task_nand tnand;
    ealib::tasks::task_and tand;
    ealib::tasks::task_ornot tornot;
    ealib::tasks::task_or tor;
    ealib::tasks::task_andnot tandnot;
    ealib::tasks::task_nor tnor;
    ealib::tasks::task_xor txor;
    ealib::tasks::task_equals tequals;
    
    // numbers 9, 10
    int x = 9;
    int y = 10;
    
    BOOST_CHECK(tnot(x, y, -10));
    BOOST_CHECK(tnot(x, y, -11));
    BOOST_CHECK(tnand(x, y, -9));
    BOOST_CHECK(tand(x, y, 8));
    BOOST_CHECK(tornot(x, y, -3));
    BOOST_CHECK(tornot(x, y, -2));
    BOOST_CHECK(tor(x, y, 11));
    BOOST_CHECK(tandnot(x, y, 1));
    BOOST_CHECK(tandnot(x, y, 2));
    BOOST_CHECK(tnor(x, y, 4294967284));
    BOOST_CHECK(txor(x, y, 3));
    BOOST_CHECK(tequals(x, y, 4294967292));
}


BOOST_AUTO_TEST_CASE(test_ea_type) {
    ea_type ea(build_md());
    generate_ancestors(repro_ancestor(), 1, ea);
    
    ea_type::individual_ptr_type p = ea.population()[0];
    ea_type::genome_type& r = p->genome();
    
    r[94] = ea.isa()["nop_c"];
    r[95] = ea.isa()["input"];
    r[96] = ea.isa()["input"];
    r[97] = ea.isa()["nand"];
    r[98] = ea.isa()["output"];
    
    p->priority() = 1.0;
    
    BOOST_CHECK(ea.population().size()==1);
    
    put<SCHEDULER_TIME_SLICE>(100,ea);
    ea.scheduler()(ea.population(),ea);
    
    BOOST_CHECK(ea.population().size()==2);
    
    for(ea_type::population_type::iterator i=ea.population().begin(); i!=ea.population().end(); ++i) {
        BOOST_CHECK(ea.population()[0]->priority() == 2.0);
    }
    
    // now let's check serialization...
    std::ostringstream out;
    checkpoint::save(out, ea);
    
    ea_type ea2;
    std::istringstream in(out.str());
    checkpoint::load(in, ea2);
    
    ea_type::individual_type& i1=*ea.population()[0];
    ea_type::individual_type& i2=*ea2.population()[0];
    
    BOOST_CHECK(i1.repr() == i2.repr());
    BOOST_CHECK(i1.hw() == i2.hw());
}


BOOST_AUTO_TEST_CASE(test_self_replication) {
    ea_type ea(build_md());
    put<MUTATION_PER_SITE_P>(0.0,ea);

    generate_ancestors(nopx_ancestor(), 1, ea);
    
    ea_type::individual_ptr_type p = ea.population()[0];
    ea_type::genome_type& r = p->repr();
    
    r[0] = ea.isa()["h_alloc"];
    r[1] = ea.isa()["nop_c"];
    r[2] = ea.isa()["nop_a"];
    r[3] = ea.isa()["h_search"];
    r[4] = ea.isa()["nop_c"];
    r[5] = ea.isa()["mov_head"];
    
    r[91] = ea.isa()["h_search"];
    r[92] = ea.isa()["h_copy"];
    r[93] = ea.isa()["nop_c"];
    r[94] = ea.isa()["nop_a"];
    r[95] = ea.isa()["if_label"];
    r[96] = ea.isa()["h_divide"];
    r[97] = ea.isa()["mov_head"];
    r[98] = ea.isa()["nop_a"];
    r[99] = ea.isa()["nop_b"];
    
    p->priority() = 1.0;
    BOOST_CHECK(ea.population().size()==1);
    
    put<SCHEDULER_TIME_SLICE>(389,ea);
    ea.scheduler()(ea.population(),ea);
    
    BOOST_CHECK(ea.population().size()==2);
    
    BOOST_CHECK(ea.population()[0]->hw().repr() == ea.population()[1]->hw().repr());
    BOOST_CHECK(ea.population()[0]->hw() == ea.population()[1]->hw());
}

BOOST_AUTO_TEST_CASE(test_al_messaging) {
    ea_type ea(build_md());
    generate_ancestors(nopx_ancestor(), 2, ea);
    
    ea_type::individual_ptr_type p = ea.population()[0];
    ea_type::genome_type& r = p->repr();
    
    r[4] = ea.isa()["nop_c"];
    r[5] = ea.isa()["input"];
    r[6] = ea.isa()["input"];
    r[7] = ea.isa()["nand"];
    r[8] = ea.isa()["bc_msg"];
    
    p->priority() = 1.0;
    BOOST_CHECK(ea.population().size()==2);
    
    ea.env().face_org(ea[0], ea[1]);
    
    put<SCHEDULER_TIME_SLICE>(100,ea);
    ea.scheduler()(ea.population(),ea);
    
    // the scheduler may shuffle the population -- so, we have to check that
    // *one of* the individuals received a message, not a particular one:
    BOOST_CHECK((ea.population()[0]->hw().msgs_queued()>0)
                || (ea.population()[1]->hw().msgs_queued()>0));
}

BOOST_AUTO_TEST_CASE(test_digevo_checkpoint) {
    ea_type ea(build_md()), ea2;
    
    generate_ancestors(repro_ancestor(), 1, ea);
    ea_type::individual_ptr_type p = ea.population()[0];
    p->priority() = 1.0;
    ea.lifecycle().advance_epoch(400,ea);
    BOOST_CHECK(ea.population().size()>1);
    
    std::ostringstream out;
    checkpoint::save(out, ea);
    
    std::istringstream in(out.str());
    checkpoint::load(in, ea2);
    
    BOOST_CHECK(ea.population() == ea2.population());
    BOOST_CHECK(ea.env() == ea2.env());
    BOOST_CHECK(ea.rng() == ea2.rng());
    
    ea.lifecycle().advance_epoch(10,ea);
    ea.lifecycle().advance_epoch(10,ea2);
    
    BOOST_CHECK(ea.population() == ea2.population());
    BOOST_CHECK(ea.env() == ea2.env());
    BOOST_CHECK(ea.rng() == ea2.rng());
}
