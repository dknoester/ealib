/* test_markov_network.cpp
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
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <mkv/markov_network.h>
#include <ea/cvector.h>

#include "test_libea.h"

/*!
 */
BOOST_AUTO_TEST_CASE(test_markov_network_update3) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace control;
    using namespace ea;
    /*
	 | 0  | 1  | 2  | 3  | rng | history
	 | 0i | 1i | 0o | 0h | 
	 t-----------------------------
	 -1| 0   0 |    |    | 5   | 
	 0 | 0   1 | 0  | 0  | 11  | 0,0
	 1 | 0   1 | 0  | 1  | 22  | 1,1
	 2 | 1   1 | 1  | 0  | 2   | 1,1
	 3 |       | 0  | 1  | 10  |
	 
	 0-in 1-in
	 |   /
	 4-HHH
	 /    \
	 2-out 3-out
	 */
    int data[33]={
		44, 255-44, // start
		1, 8, // 2in, 2out
        0, // history size (1)
        3, 3, // posf, negf
		0, 1, // inputs
		2, 3, // outputs
        32767, // poswv
        0, // negwv
		10, 0, 0, 0, // P table
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 10
	};
	
	// test cases:
	int tc0[3] = { 
		0, 0, // inputs
		0 // expected outputs
	};
	int tc1[3] = {
		0, 1,
		0
	};
	int tc2[3] = {
		0, 1,
		1
	};
	int tc3[3] = {
		1, 1,
		0
	};
	
	int* in;
	int out[1] = { 0 };
    
    markov_network mkv(2, 1, 1, 42);
    put<NODE_WV_STEPS>(32767.0, mkv);
    put<NODE_ALLOW_ZERO>(false, mkv);
    put<NODE_INPUT_FLOOR>(1, mkv);
    put<NODE_INPUT_LIMIT>(8, mkv);
    put<NODE_OUTPUT_FLOOR>(1, mkv);
    put<NODE_OUTPUT_LIMIT>(8, mkv);
    put<NODE_HISTORY_FLOOR>(1, mkv);
    put<NODE_HISTORY_LIMIT>(8, mkv);
    
    build_markov_network(mkv, data, data+33, mkv);
    
    in=tc0;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]));
	
	in=tc1;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]));

    // before learning:
    {
        adaptive_mkv_node& node = *dynamic_cast<adaptive_mkv_node*>(mkv.begin()->get());
        BOOST_CHECK(node._table(1,1)==10);
        BOOST_CHECK(node._table(1,4)==13);
        BOOST_CHECK((node._history.begin()->first==1) && (node._history.begin()->second==1));
    }    
    
	in=tc2;
    update_n(1, mkv, in, in+2, out);
    
    // after learning:
    {
        adaptive_mkv_node& node = *dynamic_cast<adaptive_mkv_node*>(mkv.begin()->get());
        BOOST_CHECK(node._table(1,1)==20);
        BOOST_CHECK(node._table(1,4)==23);
        BOOST_CHECK((node._history.begin()->first==1) && (node._history.begin()->second==2));
    }    

    BOOST_CHECK((out[0]==in[2]));
	
	in=tc3;
    update_n(1, mkv, in, in+2, out);
    BOOST_CHECK((out[0]==in[2]));
}


/*!
 */
BOOST_AUTO_TEST_CASE(test_markov_network_update2) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace control;
    using namespace ea;
	
	/*
	 | 0  | 1  | 2  | 3  | rng
	 | 0i | 1i | 0o | 1o |
	 t------------------------
	 -1| 0    0  | 0    0   5
	 0 | 0    1  | 0    0   11
	 1 | 1    0  | 0    1   13
	 2 | 1    1  | 1    1   2
	 3 |         | 0    1
	 
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
		1, 1
	};
	int tc3[4] = {
		1, 1,
		0, 1
	};
	
	int* in;
	int out[2] = { 0, 0 };
    
    markov_network mkv(2, 2, 1, 42); //random numbers == 5,11,13,2,10
    put<NODE_ALLOW_ZERO>(false, mkv);
    put<NODE_INPUT_FLOOR>(1, mkv);
    put<NODE_INPUT_LIMIT>(8, mkv);
    put<NODE_OUTPUT_FLOOR>(1, mkv);
    put<NODE_OUTPUT_LIMIT>(8, mkv);
    put<NODE_HISTORY_FLOOR>(1, mkv);
    put<NODE_HISTORY_LIMIT>(8, mkv);
    
    build_markov_network(mkv, data, data+64, mkv);
    
    in=tc0;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc1;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc2;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc3;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
}


/*!
 */
BOOST_AUTO_TEST_CASE(test_markov_network_update1) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace control;
    using namespace ea;
	
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
    
    markov_network mkv(2, 2, 1);
    put<NODE_INPUT_FLOOR>(1, mkv);
    put<NODE_INPUT_LIMIT>(8, mkv);
    put<NODE_OUTPUT_FLOOR>(1, mkv);
    put<NODE_OUTPUT_LIMIT>(8, mkv);
    put<NODE_HISTORY_FLOOR>(1, mkv);
    put<NODE_HISTORY_LIMIT>(8, mkv);
    
    build_markov_network(mkv, data, data+64, mkv);
    BOOST_CHECK(mkv.size()==1);
    BOOST_CHECK(mkv.svm_size()==5);
    
	
	int* in;
	int out[2] = { 0, 0 };
	
    {
        deterministic_mkv_node& node = *dynamic_cast<deterministic_mkv_node*>(mkv.begin()->get());
        BOOST_CHECK(node._in[0]==0);
        BOOST_CHECK(node._in[1]==1);
        BOOST_CHECK(node._out[0]==2);
        BOOST_CHECK(node._out[1]==3);
        BOOST_CHECK(node._table[0]==0);
        BOOST_CHECK(node._table[1]==1);
        BOOST_CHECK(node._table[2]==2);
        BOOST_CHECK(node._table[3]==3);
    }
    
	in=tc0;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc1;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc2;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
	
	in=tc3;
    update_n(1, mkv, in, in+2, out);
	BOOST_CHECK((out[0]==in[2]) && (out[1]==in[3]));
}


/*!
 */
BOOST_AUTO_TEST_CASE(test_probabilistic_mkv_node_ctor) {
    using namespace mkv;
	using namespace mkv::detail;
    
	int data[32]={
		42, 255-42, // start
		1, 8, // 2in, 2out (outputs are range limited, see hmm.cpp)
		0, 1, // inputs from edge 0,1
		2, 4, // outputs to edge 2,4
		10, 0, 0, 0, // P table
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 10
	};
    
    index_list_type inputs(&data[4], &data[6]);
    index_list_type outputs(&data[6], &data[8]);
    
    {
        probabilistic_mkv_node node(inputs, outputs, &data[8], true);
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==0);
        BOOST_CHECK(node._table(1,1)==10);
        BOOST_CHECK(node._table(3,2)==0);
        BOOST_CHECK(node._table(3,3)==10);
        BOOST_CHECK(node._table(2,4)==10); // row sum
    }
    {
        probabilistic_mkv_node node(inputs, outputs, &data[8], false);
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==1);
        BOOST_CHECK(node._table(1,1)==10);
        BOOST_CHECK(node._table(3,2)==1);
        BOOST_CHECK(node._table(3,3)==10);
        BOOST_CHECK(node._table(2,4)==13); // row sum
    }
}

/*!
 */
BOOST_AUTO_TEST_CASE(test_adaptive_mkv_node_ctor) {
    using namespace mkv;
	using namespace mkv::detail;
    
	int data[48]={
		44, 255-44, // start
		1, 8, // 2in, 2out (outputs are range limited, see hmm.cpp)
        3, // history size
        3, 4, // posf, negf
		0, 1, // inputs
		2, 4, // outputs
        13, 11, 12, // poswv
        19, 18, 17, // negwv
		10, 0, 0, 0, // P table
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 10
	};
    
    index_list_type inputs(&data[4], &data[6]);
    index_list_type outputs(&data[6], &data[8]);
    weight_vector_type poswv(&data[11], &data[14]);
    weight_vector_type negwv(&data[14], &data[17]);
    
    {
        adaptive_mkv_node node(data[4],
                              data[5], poswv,
                              data[6], negwv,
                              inputs, outputs, &data[17], true);
        
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==0);
        BOOST_CHECK(node._table(1,1)==10);
        BOOST_CHECK(node._table(3,2)==0);
        BOOST_CHECK(node._table(3,3)==10);
        BOOST_CHECK(node._table(2,4)==10); // row sum
    }
    {
        adaptive_mkv_node node(data[4],
                              data[5], poswv,
                              data[6], negwv,
                              inputs, outputs, &data[17], false);
        
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==1);
        BOOST_CHECK(node._table(1,1)==10);
        BOOST_CHECK(node._table(3,2)==1);
        BOOST_CHECK(node._table(3,3)==10);
        BOOST_CHECK(node._table(2,4)==13); // row sum
    }
}

/*!
 */
BOOST_AUTO_TEST_CASE(test_deterministic_mkv_node_ctor) {
    using namespace mkv;
	using namespace mkv::detail;
    
	int data[32]={
		43, 255-43, // start
		1, 1, // 2in, 2out (see hmm.cpp)
		0, 1, // inputs from edge 0,1
		2, 4, // outputs to edge 2,4
		2, // D table (these are column indices)
		1,
		3,
		0
	};
    
    index_list_type inputs(&data[4], &data[6]);
    index_list_type outputs(&data[6], &data[8]);
    deterministic_mkv_node node(inputs, outputs, &data[8]);
    
	BOOST_CHECK(node._table[0]==2);
	BOOST_CHECK(node._table[1]==1);
	BOOST_CHECK(node._table[2]==3);
	BOOST_CHECK(node._table[3]==0);
}

/*! 
 */
BOOST_AUTO_TEST_CASE(test_markov_network_ctor2) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace ea;
    
	int data[109] = {
        5, 0,
		6, 0,
		7, 0,
		8, 0, // 64
        43, 255-43, // start
		1, 1, // 2in, 2out (see hmm.cpp)
		0, 1, // inputs from edge 0,1
		2, 4, // outputs to edge 2,4
		2, // D table (these are column indices)
		1,
		3,
		0, // 76
        44, 255-44, // start
		1, 8, // 2in, 2out (outputs are range limited, see hmm.cpp)
        2, // history size
        3, 4, // posf, negf
		0, 1, // inputs
		2, 4, // outputs
        13, 11, 12, // poswv
        19, 18, 17, // negwv
		10, 0, 0, 0, // P table
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 10, // 105
        // <snip>
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
    
    markov_network mkv(2, 2, 2);
    put<NODE_WV_STEPS>(32767.0, mkv);
    put<NODE_ALLOW_ZERO>(false, mkv);
    put<NODE_INPUT_FLOOR>(1, mkv);
    put<NODE_INPUT_LIMIT>(8, mkv);
    put<NODE_OUTPUT_FLOOR>(1, mkv);
    put<NODE_OUTPUT_LIMIT>(8, mkv);
    put<NODE_HISTORY_FLOOR>(1, mkv);
    put<NODE_HISTORY_LIMIT>(8, mkv);
    
    cvector<int> cv(data, data+109);
    build_markov_network(mkv, cv.begin(), cv.end(), mkv);
    BOOST_CHECK(mkv.size()==4);
    BOOST_CHECK(mkv.svm_size()==6);
    

    {
        deterministic_mkv_node& node = *dynamic_cast<deterministic_mkv_node*>((mkv.begin())->get());
        BOOST_CHECK(node._table[0]==2);
        BOOST_CHECK(node._table[1]==1);
        BOOST_CHECK(node._table[2]==3);
    }
    {
        adaptive_mkv_node& node = *dynamic_cast<adaptive_mkv_node*>((mkv.begin()+1)->get());
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==1);
        BOOST_CHECK(node._table(2,4)==13);
    }
    {
        probabilistic_mkv_node& node = *dynamic_cast<probabilistic_mkv_node*>((mkv.begin()+2)->get());
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==1);
        BOOST_CHECK(node._table(2,4)==13);
    }
    {
        probabilistic_mkv_node& node = *dynamic_cast<probabilistic_mkv_node*>((mkv.begin()+3)->get());
        BOOST_CHECK(node._table(0,0)==1);
        BOOST_CHECK(node._table(1,0)==2);
        BOOST_CHECK(node._table(7,0)==8);
    }
}


/*! 
 */
BOOST_AUTO_TEST_CASE(test_markov_network_ctor1) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace ea;
    
	int data[109] = {
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
		4, 0,
		5, 0,
		6, 0,
		7, 0,
		8, 0, // 64
        43, 255-43, // start
		1, 1, // 2in, 2out (see hmm.cpp)
		0, 1, // inputs from edge 0,1
		2, 4, // outputs to edge 2,4
		2, // D table (these are column indices)
		1,
		3,
		0, // 76
        44, 255-44, // start
		1, 8, // 2in, 2out (outputs are range limited, see hmm.cpp)
        2, // history size
        3, 4, // posf, negf
		0, 1, // inputs
		2, 4, // outputs
        13, 11, 12, // poswv
        19, 18, 17, // negwv
		10, 0, 0, 0, // P table
		0, 10, 0, 0,
		0, 0, 10, 0,
		0, 0, 0, 10 // 105
	};
    
    markov_network mkv(2, 2, 2);
    put<NODE_WV_STEPS>(32767.0, mkv);
    put<NODE_ALLOW_ZERO>(false, mkv);
    put<NODE_INPUT_FLOOR>(1, mkv);
    put<NODE_INPUT_LIMIT>(8, mkv);
    put<NODE_OUTPUT_FLOOR>(1, mkv);
    put<NODE_OUTPUT_LIMIT>(8, mkv);
    put<NODE_HISTORY_FLOOR>(1, mkv);
    put<NODE_HISTORY_LIMIT>(8, mkv);
    
    build_markov_network(mkv, data, data+109, mkv);
    BOOST_CHECK(mkv.size()==4);
    BOOST_CHECK(mkv.svm_size()==6);
    
    {
        probabilistic_mkv_node& node = *dynamic_cast<probabilistic_mkv_node*>(mkv.begin()->get());
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==1);
        BOOST_CHECK(node._table(2,4)==13);
    }
    {
        probabilistic_mkv_node& node = *dynamic_cast<probabilistic_mkv_node*>((mkv.begin()+1)->get());
        BOOST_CHECK(node._table(0,0)==1);
        BOOST_CHECK(node._table(1,0)==2);
        BOOST_CHECK(node._table(7,0)==8);
    }
    {
        deterministic_mkv_node& node = *dynamic_cast<deterministic_mkv_node*>((mkv.begin()+2)->get());
        BOOST_CHECK(node._table[0]==2);
        BOOST_CHECK(node._table[1]==1);
        BOOST_CHECK(node._table[2]==3);
    }
    {
        adaptive_mkv_node& node = *dynamic_cast<adaptive_mkv_node*>((mkv.begin()+3)->get());
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==1);
        BOOST_CHECK(node._table(2,4)==13);
    }
}
