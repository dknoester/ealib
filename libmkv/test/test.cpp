/* test.cpp
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester.
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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <iostream>
#include <ea/markov_network.h>
#include <ea/cvector.h>
#include "test.h"

/*! Test for logic gates.
 */
BOOST_AUTO_TEST_CASE(test_logic_gate) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace ea;
	
    //	 | 0  | 1  | 2  | 3  | 4  |
    //	 | 0i | 1i | 0o | 1o | 0h |
    //
    //	 0h:
    //	 o(2,5) = f(i(0,1)); o(2,5)==i(0,1)
    //	 1h:
    //	 o(3) = f(i(4,5,1)); o(3)==i(1)
    //
    //	 0-in 1-in
    //	 |   /
    //	 4-HHH
    //	 /    \
    //	 2-out 3-out
    
	int data[64] = {
		43, 255-43, // start
		1, 1, // 2in, 2out
		5, 1, // inputs from node 0,1 (mod 5)
		2, 3, // outputs to node 2,3
		0, // d table; echos input
		1,
		2,
		3,
		0, 0, 0, 0, //16
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};
	
    meta_data md;
    put<MKV_GATE_TYPES>("logic", md);
    put<GATE_INPUT_FLOOR>(1, md);
    put<GATE_INPUT_LIMIT>(8, md);
    put<GATE_OUTPUT_FLOOR>(1, md);
    put<GATE_OUTPUT_LIMIT>(8, md);
    
    markov_network net(2, 2, 1);
    build_markov_network(net, data, data+64, md);
    BOOST_CHECK(net.size()==1);
    BOOST_CHECK(net.nstates()==5);
    
    logic_gate& g = boost::get<logic_gate>(net[0]);
    BOOST_CHECK(g.inputs[0]==0);
    BOOST_CHECK(g.inputs[1]==1);
    BOOST_CHECK(g.outputs[0]==2);
    BOOST_CHECK(g.outputs[1]==3);
    BOOST_CHECK(g.M[0]==0);
    BOOST_CHECK(g.M[1]==1);
    BOOST_CHECK(g.M[2]==2);
    BOOST_CHECK(g.M[3]==3);
    
    // test cases:
	int tc0[4] = {
		0, 0, // inputs
		0, 0 // expected outputs
	};
	int tc1[4] = {
		0, 1,
		0, 1
	};
	int tc2[4] = {
		1, 0,
		1, 0
	};
	int tc3[4] = {
		1, 1,
		1, 1
	};
    
    int* in;
	int out[2] = { 0, 0 };
    
	in=tc0;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
    
	in=tc1;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc2;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc3;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
}


/*! Test for Markov gates.
 */
BOOST_AUTO_TEST_CASE(test_markov_network_update2) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace ea;
	
    //	 | 0  | 1  | 2  | 3  | rng
    //	 | 0i | 1i | 0o | 1o |
    //	 t------------------------
    //	 -1| 0    0  | 0    0   5
    //	 0 | 0    1  | 0    0   11
    //	 1 | 1    0  | 0    1   13
    //	 2 | 1    1  | 1    1   2
    //	 3 |         | 0    1
    //
    //	 0-in 1-in
    //	 |   /
    //	 4-HHH
    //	 /    \
    //	 2-out 3-out
    
	int data[64] = {
		42, 255-42, // start
		1, 8, // 2in, 2out (see hmm.cpp)
		0, 1, // inputs from edge 0,1
		2, 3, // outputs to edge 2,3
		10, 0, 0, 0, // P table
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 10, // 24
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};
	
    meta_data md;
    put<MKV_GATE_TYPES>("markov", md);
    put<GATE_INPUT_FLOOR>(1, md);
    put<GATE_INPUT_LIMIT>(8, md);
    put<GATE_OUTPUT_FLOOR>(1, md);
    put<GATE_OUTPUT_LIMIT>(8, md);
    
    markov_network net(2, 2, 1, 42);

    build_markov_network(net, data, data+64, md);
    BOOST_CHECK(net.size()==1);
    BOOST_CHECK(net.nstates()==5);
    
    markov_gate& g = boost::get<markov_gate>(net[0]);
    BOOST_CHECK(g.inputs[0]==0);
    BOOST_CHECK(g.inputs[1]==1);
    BOOST_CHECK(g.outputs[0]==2);
    BOOST_CHECK(g.outputs[1]==3);
    BOOST_CHECK(g.M(0,0)==1.0);
    BOOST_CHECK(g.M(1,1)==1.0);
    BOOST_CHECK(g.M(2,2)==1.0);
    BOOST_CHECK(g.M(3,3)==1.0);
    
    // test cases:
    // first four random numbers:
    //    0.37454
    //    0.796543
    //    0.950714
    //    0.183435
    
	int tc0[4] = {
		0, 0, // inputs
		0, 0 // expected outputs
	};
	int tc1[4] = {
		0, 1,
		0, 1
	};
	int tc2[4] = {
		1, 0,
		1, 0
	};
	int tc3[4] = {
		1, 1,
		1, 1
	};
	
    int* in;
	int out[2] = { 0, 0 };
    
	in=tc0;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
    
	in=tc1;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc2;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc3;
    update(net, 1, in, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
}


/*! Tests for a mixed-gate Markov network.
 */
BOOST_AUTO_TEST_CASE(test_markov_network_ctor2) {
    using namespace mkv;
    using namespace mkv::detail;
    using namespace ea;
    
    int data[76] = {
        5, 0,
        6, 0,
        7, 0,
        8, 0, // 64
        43, 255-43, // start
        1, 1, // 2in, 2out
        0, 1, // inputs from edge 0,1
        2, 4, // outputs to edge 2,4
        2, // D table (these are column indices)
        1,
        3,
        0, // 76
        42, 255-42, // start
        1, 1, // 2in, 2out
        6, 1, // inputs from edge 0,1 (mod 6)
        2, 4, // outputs to edge 2,4
        10, 0, 0, 0, // P table
        0, 10, 0, 0,
        0, 0, 10, 0,
        0, 0, 0, 10, // 24
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0, // 40
        42, 255-42, // start
        2, 0, // 3in, 1out
        2, 4, 1, // inputs from edge 2,4,1
        3, // outputs to edge 3 // 48
        1, 0, // P table
        2, 0,
        3, 0,
        4, 0
    };
    
    meta_data md;
    put<MKV_GATE_TYPES>("logic,markov", md);
    put<GATE_INPUT_FLOOR>(1, md);
    put<GATE_INPUT_LIMIT>(8, md);
    put<GATE_OUTPUT_FLOOR>(1, md);
    put<GATE_OUTPUT_LIMIT>(8, md);
    
    // build the network, check that the gate is what we expect:
    markov_network net(2, 2, 2, 42);
    cvector<int> cv(data, data+76);
    build_markov_network(net, cv.begin(), cv.end(), md);
    BOOST_CHECK(net.size()==3);
    BOOST_CHECK(net.nstates()==6);
    
    {
        logic_gate& g = boost::get<logic_gate>(net[0]);
        BOOST_CHECK(g.inputs.size() == 2);
        BOOST_CHECK(g.outputs.size() == 2);
        BOOST_CHECK(g.inputs[0]==0);
        BOOST_CHECK(g.inputs[1]==1);
        BOOST_CHECK(g.outputs[0]==2);
        BOOST_CHECK(g.outputs[1]==4);
        BOOST_CHECK(g.M[0]==2);
        BOOST_CHECK(g.M[1]==1);
        BOOST_CHECK(g.M[2]==3);
        BOOST_CHECK(g.M[3]==0);
    }
    
    {
        markov_gate& g = boost::get<markov_gate>(net[1]);
        BOOST_CHECK(g.inputs.size() == 2);
        BOOST_CHECK(g.outputs.size() == 2);
        BOOST_CHECK(g.inputs[0]==0);
        BOOST_CHECK(g.inputs[1]==1);
        BOOST_CHECK(g.outputs[0]==2);
        BOOST_CHECK(g.outputs[1]==4);
        BOOST_CHECK(g.M(0,0)==1.0);
        BOOST_CHECK(g.M(1,1)==1.0);
        BOOST_CHECK(g.M(2,2)==1.0);
        BOOST_CHECK(g.M(3,3)==1.0);
    }
    
    {
        markov_gate& g = boost::get<markov_gate>(net[2]);
        BOOST_CHECK(g.inputs.size() == 3);
        BOOST_CHECK(g.outputs.size() == 1);
        BOOST_CHECK(g.inputs[0]==2);
        BOOST_CHECK(g.inputs[1]==4);
        BOOST_CHECK(g.inputs[2]==1);
        BOOST_CHECK(g.outputs[0]==3);
        
        BOOST_CHECK(g.M(0,0)==1.0);
        BOOST_CHECK(g.M(1,0)==1.0);
        BOOST_CHECK(g.M(2,1)==0.0);
        BOOST_CHECK(g.M(3,1)==0.0);
        BOOST_CHECK(g.M(4,0)==1.0);
        BOOST_CHECK(g.M(5,0)==1.0);
        BOOST_CHECK(g.M(6,1)==0.0);
        BOOST_CHECK(g.M(7,1)==0.0);
    }
}
