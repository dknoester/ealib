#include "test.h"

#include <ea/artificial_life/artificial_life.h>
#include <ea/artificial_life/hardware.h>
#include <ea/artificial_life/isa.h>
#include <ea/artificial_life/topology.h>

typedef artificial_life<
hardware,
isa
> al_type;


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
    al_type::individual_ptr_type p = make_individual_ptr(al_type::individual_type(),al);    
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


BOOST_AUTO_TEST_CASE(test_well_mixed_topo) {
    //well_mixed topo;
    // test topo
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
    // test initialization
    // test serialization
}
