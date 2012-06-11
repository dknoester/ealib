/* test_update.cpp
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
#include <boost/test/unit_test.hpp>
#include <fn/hmm/hmm_network.h>
#include <fn/hmm/deterministic_node.h>
#include "test.h"

/*! Tests that a simple HMM network is correctly constructed and updated.
 */
BOOST_AUTO_TEST_CASE(test_simple_deterministic_hmm_network_update) {
	using namespace fn::hmm;
	using namespace fn::hmm::detail;
	test_rng rng(4);
	
	/*
	 | 0  | 1  | 2  | 3  | 4  |
	 | 0i | 1i | 0o | 1o | 0h |
	 
	 0h:
	 o(2,5) = f(i(0,1)); o(2,5)==i(0,1)
	 1h:
	 o(3) = f(i(4,5,1)); o(3)==i(1)
	 
	 0-in 1-in
	 |   /
	 4-HHH
	 /    \
	 2-out 3-out
	 */	
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
	
	hmm_network hmm(genome(data,data+64), 2, 2, 1);
	BOOST_CHECK(hmm.num_nodes()==1);
	BOOST_CHECK(hmm.num_states()==5);
	
	deterministic_node* node=dynamic_cast<deterministic_node*>(hmm.node(0));
	BOOST_CHECK(node->num_inputs()==2);
	BOOST_CHECK(node->num_outputs()==2);	
	BOOST_CHECK(node->xinput(0)==0);
	BOOST_CHECK(node->xoutput(1)==3);
	BOOST_CHECK(node->dtable(0)==0);
	BOOST_CHECK(node->dtable(1)==1);
	BOOST_CHECK(node->dtable(2)==2);	
	BOOST_CHECK(node->dtable(3)==3);
	
	in=tc0;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc1;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc2;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc3;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
}


/*! Tests that a simple HMM network is correctly constructed and updated.
 */
BOOST_AUTO_TEST_CASE(test_simple_probabilistic_hmm_network_update) {
	using namespace fn::hmm;
	using namespace fn::hmm::detail;
	test_rng rng(2);
	
	/*
	 | 0  | 1  | 2  | 3  | rng
	 | 0i | 1i | 0o | 1o |
	 t------------------------
	 -1| 0    0  | 0    0   2
	 0 | 0    1  | 0    0   2
	 1 | 1    0  | 0    1   2
	 2 | 1    1  | 0    1   4
	 3 |         | 1    1
	 
	 0-in 1-in
	 |   /
	 4-HHH
	 /    \
	 2-out 3-out
	 */
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
		0, 1
	};
	int tc3[4] = {
		1, 1,
		1, 1
	};
	
	int* in;
	int out[2] = { 0, 0 };
	
	hmm_network hmm(genome(data,data+64), 2, 2, 1);
	BOOST_CHECK(hmm.num_nodes()==1);
	BOOST_CHECK(hmm.num_states()==5);
	
	in=tc0;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc1;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc2;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc3;
	rng.reset(4);
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
}


/*! Tests that a complex HMM network is correctly constructed and updated.
 
 Note that since the test cases are executed serially, there *is* state information
 from the previous time step that must be factored in.  See the table below.
 */
BOOST_AUTO_TEST_CASE(test_complex_hmm_network_update) {
	using namespace fn::hmm;
	using namespace fn::hmm::detail;
	test_rng rng(4);
	
	/*
	 | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  |
	 | 0i | 1i | 0o | 1o | 0h | 1h | 2h | 3h |
	 t------------------------------------------
	 -1| 0    0	 | 0    0    0    0    0    0
	 0 | 0    1	 | 0    0    0    0    1    1
	 1 | 1    0	 | 1    1    0    1    1    0
	 2 | 1    1	 | 0    0    0    0    1    1
	 3 |       	 | 1    1    0    1    1    0
	 
	 update rules:
	 6,5 <-- 0,1
	 6,7 <-- -4,-1
	 2,7 <-- 4,5
	 2,3 <-- xor(6,5)
	 
	 0-in 1-in 0,0
	 |   / | 0,0,0
	 4-HHH | 
     |   \ |
	 |  5-HHH
	 | /   |
	 6-HHH |
	 |  \  |
	 |  7-HHH
	 |   / | 
	 2-out 3-out
	 */	
	int data[64] = {
		// hidden node 4 -- echos input
		43, 255-43, // start
		1, 1, // 2in, 2out
		0, 1, // inputs from node 0,1
		6, 5, // outputs to node 6,5
		0, // d table
		1,
		2,
		3,
		// hidden node 5 -- inverts input
		43, 255-43, // start
		1, 1, // 2in, 2out
		4, 1, // inputs from node 4,1
		6, 7, // outputs to node 6,7
		3, // d table
		2,
		1,
		0,
		// hidden node 6 -- echos input
		43, 255-43, // start
		1, 1, // 2in, 2out
		4, 5, // inputs from node 4,5
		2, 7, // outputs to node 2,7
		0, // d table
		1,
		2,
		3,
		// hidden node 7 -- xors input, outputs to both
		43, 255-43, // start
		1, 1, // 2in, 2out
		6, 5, // inputs from node 6,5
		2, 3, // outputs to node 2,3
		0, // d table
		3,
		3,
		0,		
		0, 0, 0, 0, // 48
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	};
	
	// test cases
	int tc0[4] = { 
		0, 0, // inputs
		0, 0 // expected outputs
	};
	int tc1[4] = {
		0, 1,
		1, 1
	};
	int tc2[4] = {
		1, 0,
		0, 0
	};
	int tc3[4] = {
		1, 1,
		1, 1
	};
	
	int* in;
	int out[2] = { 0, 0 };
	
	hmm_network hmm(genome(data,data+64), 2, 2, 6);
	BOOST_CHECK(hmm.num_nodes()==4);
	BOOST_CHECK(hmm.num_states()==10);
	
	deterministic_node* node=dynamic_cast<deterministic_node*>(hmm.node(3));
	BOOST_CHECK(node->num_inputs()==2);
	BOOST_CHECK(node->num_outputs()==2);	
	BOOST_CHECK(node->xinput(0)==6);
	BOOST_CHECK(node->xoutput(1)==3);
	BOOST_CHECK(node->dtable(0)==0);
	BOOST_CHECK(node->dtable(1)==3);
	BOOST_CHECK(node->dtable(2)==3);	
	BOOST_CHECK(node->dtable(3)==0);
	
	in=tc0;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc1;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc2;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc3;
	hmm.update_n(1,in,in+2,out,rng);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
}
