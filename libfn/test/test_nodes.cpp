#include <iostream>
#include <boost/test/unit_test.hpp>
#include <fn/hmm/hmm_node.h>
#include <fn/hmm/probabilistic_node.h>
#include <fn/hmm/deterministic_node.h>
#include "test.h"

/*! Tests that a probabilistic HMM node is properly constructed from its genome.
 */
BOOST_AUTO_TEST_CASE(test_probabilistic_node_ctor) {
	using namespace fn::hmm;
	using namespace fn::hmm::detail;
	test_rng rng(4);
	
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
	
	int where[64];
	memset(where,0x0f,sizeof(where));
	
	genome g(data,data+32);
	BOOST_CHECK(hmm_node::start_codon(g,0).first==true);
	BOOST_CHECK(hmm_node::start_codon(g,0).second==(sizeof(int)*(2+ // start codon
                                                                 2+ // nin, nout
                                                                 3+ // offset of inputs, outputs, table
                                                                 4+ // indices of nin & nout
                                                                 1+ // ncol
                                                                 20))); // ptable
	probabilistic_node node(g, 0, where);
	BOOST_CHECK(node.num_inputs()==2);
	BOOST_CHECK(node.num_outputs()==2);	
	BOOST_CHECK(node.xinput(0)==0);
	BOOST_CHECK(node.xoutput(1)==4);
	BOOST_CHECK(node.ptable(0,0)==10);
	BOOST_CHECK(node.ptable(0,1)==1);
	BOOST_CHECK(node.ptable(0,4)==13);	
	BOOST_CHECK(node.ptable(3,3)==10);
	BOOST_CHECK(where[32]==0x0f0f0f0f);
}

/*! Tests that a deterministic HMM node is properly constructed from its genome.
 */
BOOST_AUTO_TEST_CASE(test_deterministic_node_ctor) {
	using namespace fn::hmm;
	using namespace fn::hmm::detail;
	
	int data[32]={
		43, 255-43, // start
		1, 1, // 2in, 2out (see hmm.cpp)
		0, 1, // inputs from edge 0,1
		2, 4, // outputs to edge 2,4
		2, // D table (these are column indices)
		1,
		7,
		0
	};
    
	genome g(data,data+32);
	BOOST_CHECK(hmm_node::start_codon(g,0).first==true);
	BOOST_CHECK(hmm_node::start_codon(g,0).second==(sizeof(int)*(2+ // start codon
                                                                 2+ // nin, nout
                                                                 3+ // offset of inputs, outputs, table
                                                                 4+ // indices of nin & nout
                                                                 1+ // ncol
                                                                 4))); // dtable
	deterministic_node node(g, 0);
	BOOST_CHECK(node.num_inputs()==2);
	BOOST_CHECK(node.num_outputs()==2);	
	BOOST_CHECK(node.xinput(0)==0);
	BOOST_CHECK(node.xoutput(1)==4);
	BOOST_CHECK(node.dtable(0)==2);
	BOOST_CHECK(node.dtable(2)==3);
}


/*! Tests that a generic HMM node can be constructed from its genome.
 */
BOOST_AUTO_TEST_CASE(test_hmm_node_ctor) {
	using namespace fn::hmm;
	using namespace fn::hmm::detail;
	test_rng rng(4);
    
	int data[32]={
		43, 255-43, // start
		1, 1, // 2in, 2out
		0, 1, // inputs from edge 0,1
		2, 4, // outputs to edge 2,4
		2, // D table (these are column indices)
		1,
		7,
		0
	};
	
	genome g(data,data+32);
	BOOST_CHECK(hmm_node::start_codon(g,0).first==true);
    
	boost::shared_ptr<hmm_node> node(hmm_node::build_node(g,0));
	BOOST_CHECK(node->num_inputs()==2);
	BOOST_CHECK(node->num_outputs()==2);
}
