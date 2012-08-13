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
#include <ea/digital_evolution/spatial.h>


typedef digital_evolution<
abstract_configuration,
spatial
> al_type;


/*!
 */
BOOST_AUTO_TEST_CASE(test_avida_hardware) {
    hardware::representation_type r; // is a circular genome...
    r.push_back(0);
    r.push_back(1);
    r.push_back(2);
    r.push_back(3);
    r.push_back(4);
    r.push_back(5);
    r.push_back(6);
    r.push_back(7);

    hardware hw(r);
    
    // Check setting and getting of register values
    hw.setRegValue(hardware::AX, 27);
	BOOST_CHECK_EQUAL(hw.getRegValue(hardware::AX), 27);
    
    // Check setting and getting of head locations
    hw.setHeadLocation(hardware::IP, 5); 
	BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::IP), 5);
    
    // Advance a head location - should deal with circular genome
    hw.advanceHead(hardware::FH, 12); 
	BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::FH), 4);
    
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
    al_type al;    
    al_type::individual_ptr_type p = al.make_individual(al_type::representation_type());
    al_type::isa_type& hw_isa=al.isa();
    hardware::representation_type r; // is a circular genome...
    r.push_back(7); // 0
    r.push_back(7); // 1
    r.push_back(7); // 2
    r.push_back(7); // 3
    r.push_back(0); // 4
    r.push_back(1); // 5
    r.push_back(2); // 6
    r.push_back(3); // 7
    r.push_back(4); // 8
    r.push_back(5); // 9
    r.push_back(6); // 10
    r.push_back(7); // 11
    
    hardware hw(r);
    
    // Check if mov-head does in fact move the IP head to the position
    // before the FH (counting on the advance mechanism to put them
    // in the same place
    // FH at 10
    hw.setHeadLocation(hardware::FH, 10); 
    int cycle_cost = hw_isa(4, hw, p, al);
	BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::IP), 9);
    
    
    
    // Check h-search when complement is found 
    // Create our label.
    hw.pushLabelStack(hardware::NOP_C);
    hw.pushLabelStack(hardware::NOP_A);
    hw.pushLabelStack(hardware::NOP_B);
    cycle_cost = hw_isa(6, hw, p, al); 
    
    // If a complement is found, BX is set to the distance to the complement, 
    BOOST_CHECK_EQUAL(hw.getRegValue(hardware::BX), 7);
    
    // CX is set to its size, 
    BOOST_CHECK_EQUAL(hw.getRegValue(hardware::CX), 3);

    // and the flow head is set to the instruction immediately 
    // following the complement.  
    BOOST_CHECK_EQUAL(hw.getHeadLocation(hardware::FH), 7);    
}


BOOST_AUTO_TEST_CASE(test_self_replicator_instructions) {    
    al_type al;    
    al_type::isa_type& hw_isa=al.isa();
    
    put<POPULATION_SIZE>(100,al);
	put<REPRESENTATION_SIZE>(100,al);
	put<MUTATION_PER_SITE_P>(0.0075,al);
    put<SPATIAL_X>(10,al);
    put<SPATIAL_Y>(10,al);
    put<MUTATION_UNIFORM_INT_MIN>(0,al);
    put<MUTATION_UNIFORM_INT_MAX>(20,al);
    
    al.initialize();
    
    al_type::population_type ancestral;
    al_type::individual_type a = al_type::individual_type();
    a.name() = next<INDIVIDUAL_COUNT>(al);
    a.generation() = -1.0;
    a.update() = al.current_update();
    ancestral.append(make_population_entry(a,al));
    
    al.population().clear();
    
    al_type::representation_type r; // is a circular genome...
    r.resize(100);
    
    std::fill(r.begin(), r.end(), 3); // fill it with nop-Xs..

    r[10] = 11; // input

    al.population().append(make_population_entry(r,al));

    int x = al.population().size();

    al_type::individual_ptr_type ind1 = al.population()[0];
    
    // Check that h_alloc increases memory by 150%
    int cycle_cost = hw_isa(25, ind1->hw(), ind1, al);
    BOOST_CHECK_EQUAL(ind1->repr().size(), 250);
    
    // Check that h_copy: 
    // (1) Copies the instruction at the read head to the location pointed to by 
    // the write head
    ind1->hw().setHeadLocation(hardware::RH, 10); 
    ind1->hw().setHeadLocation(hardware::WH, 20);
    
    cycle_cost = hw_isa(26, ind1->hw(), ind1, al);
    int val = ind1->repr()[20]; 
	BOOST_CHECK_EQUAL(ind1->repr()[20], 11);

    // (2) Advances the read and write head positions by 1.
	BOOST_CHECK_EQUAL(ind1->hw().getHeadLocation(hardware::RH), 11);
	BOOST_CHECK_EQUAL(ind1->hw().getHeadLocation(hardware::WH), 21);



}

BOOST_AUTO_TEST_CASE(test_logic9_environment) {
    // test input/output
    ea::tasks::task_not tnot; 
    ea::tasks::task_nand tnand;
    ea::tasks::task_and tand;
    ea::tasks::task_ornot tornot;
    ea::tasks::task_or tor;
    ea::tasks::task_andnot tandnot;
    ea::tasks::task_nor tnor;
    ea::tasks::task_xor txor;
    ea::tasks::task_equals tequals;
    
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
    
    // test resources
}


BOOST_AUTO_TEST_CASE(test_al_type) {
    al_type al;    
    add_task<tasks::task_nand,resources::unlimited,catalysts::additive<1> >("nand", al); //1

    put<POPULATION_SIZE>(100,al);
	put<REPRESENTATION_SIZE>(100,al);
	put<MUTATION_PER_SITE_P>(0.0,al);
    put<SPATIAL_X>(10,al);
    put<SPATIAL_Y>(10,al);
    put<MUTATION_UNIFORM_INT_MIN>(0,al);
    put<MUTATION_UNIFORM_INT_MAX>(25,al);

    al.initialize();
    
    al_type::population_type ancestral;
    al_type::individual_type a = al_type::individual_type();
    a.name() = next<INDIVIDUAL_COUNT>(al);
    a.generation() = -1.0;
    a.update() = al.current_update();
    ancestral.append(make_population_entry(a,al));
    
    al.population().clear();
   
    al_type::representation_type r; // is a circular genome...
    r.resize(100);
    std::fill(r.begin(), r.end(), 11); // fill it with inputs..
    // input*90
    // 91: nopc
    // 92: input
    // 93: nand
    // 94: output
    // 95: repro
    
    r[95] = 2; // cx, input
    r[96] = 11; // input
    r[97] = 7; // nand
    r[98] = 12; // output
    r[99] = 24; // repro
    al.population().append(make_population_entry(r,al));
    
    for(al_type::population_type::iterator i=al.population().begin(); i!=al.population().end(); ++i) {
        al.events().inheritance(ancestral,ind(i,al),al);
        al.env().insert(ptr(i,al));
        ind(i,al).priority() = 1.0;
    }
    BOOST_CHECK(al.population().size()==1);
    
    put<SCHEDULER_TIME_SLICE>(100,al);
    al.scheduler()(al.population(),al);
    
    BOOST_CHECK(al.population().size()==2);
    
    for(al_type::population_type::iterator i=al.population().begin(); i!=al.population().end(); ++i) {
        BOOST_CHECK(al.population()[0]->priority() == 2.0);
    }
    
    // now let's check serialization...
    std::ostringstream out;
    checkpoint_save(al, out);
    
    al_type al2;
    std::istringstream in(out.str());
    checkpoint_load(al2, in);
    
    al_type::individual_type& i1=*al.population()[0];
    al_type::individual_type& i2=*al2.population()[0];
    
    BOOST_CHECK(i1.repr() == i2.repr());
    BOOST_CHECK(i1.hw() == i2.hw());    
}


BOOST_AUTO_TEST_CASE(test_self_replication) {
    al_type al;    
    add_task<tasks::task_nand,resources::unlimited,catalysts::additive<1> >("nand", al); //1
    
    put<POPULATION_SIZE>(100,al);
	put<REPRESENTATION_SIZE>(100,al);
	put<MUTATION_PER_SITE_P>(0.0,al);
    put<SPATIAL_X>(10,al);
    put<SPATIAL_Y>(10,al);
    put<MUTATION_UNIFORM_INT_MIN>(0,al);
    put<MUTATION_UNIFORM_INT_MAX>(25,al);
    
    al.initialize();
    
    al_type::population_type ancestral;
    al_type::individual_type a = al_type::individual_type();
    a.name() = next<INDIVIDUAL_COUNT>(al);
    a.generation() = -1.0;
    a.update() = al.current_update();
    ancestral.append(make_population_entry(a,al));
    
    al.population().clear();
    
    al_type::representation_type r; // is a circular genome...
    r.resize(100);
    std::fill(r.begin(), r.end(), 3); // fill it with nopx
   
    r[0] = 25; // h_alloc
    r[1] = 2; // nopc
    r[2] = 0; // nopa
    r[3] = 6; // hsearch
    r[4] = 2; // nopc
    r[5] = 4; // movhead
    
    r[91] = 6; // hsearch
    r[92] = 26; // hcopy
    r[93] = 2; // nopc
    r[94] = 0; // nopa
    r[95] = 5; // iflabel
    r[96] = 27; // hdivide
    r[97] = 4; // movhead
    r[98] = 0; // nopa
    r[99] = 1; // nopb
    

    al.population().append(make_population_entry(r,al));
    
    for(al_type::population_type::iterator i=al.population().begin(); i!=al.population().end(); ++i) {
        al.events().inheritance(ancestral,ind(i,al),al);
        al.env().insert(ptr(i,al));
        ind(i,al).priority() = 1.0;
    }
    BOOST_CHECK(al.population().size()==1);
    
    put<SCHEDULER_TIME_SLICE>(389,al);
    al.scheduler()(al.population(),al);
    
    BOOST_CHECK(al.population().size()==2);

    BOOST_CHECK(al.population()[0]->hw().repr() == al.population()[1]->hw().repr());
    BOOST_CHECK(al.population()[0]->hw() == al.population()[1]->hw());
}



BOOST_AUTO_TEST_CASE(test_al_messaging) {
    al_type al;    
    
    put<POPULATION_SIZE>(100,al);
	put<REPRESENTATION_SIZE>(100,al);
	put<MUTATION_PER_SITE_P>(0.0075,al);
    put<SPATIAL_X>(10,al);
    put<SPATIAL_Y>(10,al);
    put<MUTATION_UNIFORM_INT_MIN>(0,al);
    put<MUTATION_UNIFORM_INT_MAX>(20,al);
    
    al.initialize();
    
    al_type::population_type ancestral;
    al_type::individual_type a = al_type::individual_type();
    a.name() = next<INDIVIDUAL_COUNT>(al);
    a.generation() = -1.0;
    a.update() = al.current_update();
    ancestral.append(make_population_entry(a,al));
    
    al.population().clear();
    
    al_type::representation_type r; // is a circular genome...
    r.resize(100);
    std::fill(r.begin(), r.end(), 11); // fill it with inputs..
    // input*90
    // 91: nopc
    // 92: input
    // 93: nand
    // 94: tx-msg
    // 95: nopx
    
    r[95] = 2; // cx, input
    r[96] = 11; // input
    r[97] = 7; // nand
    r[98] = 17; // tx-msg
    r[99] = 3; // nopx
    al.population().append(make_population_entry(r,al));
    al.population().append(make_population_entry(r,al));
    
    for(al_type::population_type::iterator i=al.population().begin(); i!=al.population().end(); ++i) {
        al.events().inheritance(ancestral,ind(i,al),al);
        al.env().insert(ptr(i,al));
        ind(i,al).priority() = 1.0;
    }
    BOOST_CHECK(al.population().size()==2);
    
    put<SCHEDULER_TIME_SLICE>(100,al);
    al.scheduler()(al.population(),al);
    
    BOOST_CHECK(al.population()[1]->hw().msgs_queued()==1);    
}
