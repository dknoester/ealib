#include <boost/test/unit_test.hpp>
#include <fn/hmm/hmm_network.h>
#include <fn/hmm/probabilistic_node.h>
#include "test.h"

/*! Tests that HMM networks can be concatenated together.
 
 Here we take two HMM networks and concatenate them together into a
 single network.  In this example, they should each behave independently of the
 other.
 */
BOOST_AUTO_TEST_CASE(test_hmm_network_cat) {
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
	
    const int tc_size=8;
	// test cases:
	int tc0[tc_size] = { 
		0, 0, 0, 0, // inputs
		0, 0, 0, 0 // expected outputs
	};
	int tc1[tc_size] = {
		0, 1, 1, 0,
		0, 1, 0, 1
	};
	int tc2[tc_size] = {
		1, 0, 0, 1,
		0, 1, 0, 1
	};
	int tc3[tc_size] = {
		1, 1, 1, 1,
		1, 1, 1, 1
	};
	
	int* in;
	int out[4] = { 0, 0, 0, 0 };
	
	hmm_network hmm(genome(data,data+64), 2, 2, 1);
    hmm_network hmm1(genome(data,data+64), 2, 2, 1);
    hmm += hmm1;
    
	BOOST_CHECK(hmm.num_nodes()==2);
	BOOST_CHECK(hmm.num_states()==10);
    BOOST_CHECK(hmm.num_inputs()==4);
    BOOST_CHECK(hmm.num_outputs()==4);    
    BOOST_CHECK(hmm.num_hidden()==2);
	
	in=tc0;
	hmm.update_n(1,in,in+4,out,rng);
    for(int i=0; i<(tc_size/2); ++i) { 
        BOOST_CHECK(out[i]==in[i+tc_size/2]);
    }
    
	in=tc1;
	hmm.update_n(1,in,in+4,out,rng);
    for(int i=0; i<(tc_size/2); ++i) { 
        BOOST_CHECK(out[i]==in[i+tc_size/2]);
    }
	
	in=tc2;
	hmm.update_n(1,in,in+4,out,rng);
    for(int i=0; i<(tc_size/2); ++i) {
        BOOST_CHECK(out[i]==in[i+tc_size/2]); 
    }
	
	in=tc3;
	rng.reset(4);
	hmm.update_n(1,in,in+4,out,rng);
    for(int i=0; i<(tc_size/2); ++i) {
        BOOST_CHECK(out[i]==in[i+tc_size/2]); 
    }
}

/*! Tests that HMM networks can be multiplied.
 
 Here we take an HMM network and multiply it out to create a larger HMM network.
 */
BOOST_AUTO_TEST_CASE(test_hmm_network_mult) {
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
	
    const int tc_size=16;
	// test cases:
	int tc0[tc_size] = { 
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int tc1[tc_size] = {
		0, 1, 0, 1, 0, 1, 0, 1,
		0, 1, 0, 1, 0, 1, 0, 1
	};
	int tc2[tc_size] = {
		1, 0, 1, 0, 1, 0, 1, 0,
		0, 1, 0, 1, 0, 1, 0, 1
	};
	int tc3[tc_size] = {
		1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
	};
	
	int* in;
	int out[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	
	hmm_network hmm(genome(data,data+64), 2, 2, 1);
    hmm *= 4;
    
	BOOST_CHECK(hmm.num_nodes()==4);
	BOOST_CHECK(hmm.num_states()==20);
    BOOST_CHECK(hmm.num_inputs()==8);
    BOOST_CHECK(hmm.num_outputs()==8);    
    BOOST_CHECK(hmm.num_hidden()==4);
	
	in=tc0;
	hmm.update_n(1,in,in+8,out,rng);
    for(int i=0; i<(tc_size/2); ++i) { 
        BOOST_CHECK(out[i]==in[i+tc_size/2]);
    }
    
	in=tc1;
	hmm.update_n(1,in,in+8,out,rng);
    for(int i=0; i<(tc_size/2); ++i) { 
        BOOST_CHECK(out[i]==in[i+tc_size/2]);
    }
	
	in=tc2;
	hmm.update_n(1,in,in+8,out,rng);
    for(int i=0; i<(tc_size/2); ++i) {
        BOOST_CHECK(out[i]==in[i+tc_size/2]); 
    }
	
	in=tc3;
	rng.reset(4);
	hmm.update_n(1,in,in+8,out,rng);
    for(int i=0; i<(tc_size/2); ++i) {
        BOOST_CHECK(out[i]==in[i+tc_size/2]); 
    }
}