#include <boost/test/unit_test.hpp>
#include <fn/hmm/hmm_network.h>
#include <fn/hmm/probabilistic_node.h>
#include "test.h"

/*! Tests that an HMM network is properly constructed from its genome.
 */
BOOST_AUTO_TEST_CASE(test_hmm_network_ctor) {
	using namespace fn::hmm;
	using namespace fn::hmm::detail;
	test_rng rng(4);
    
	/* NOTE: The data below wraps around, as it is assumed that genomes are circular!
	 
	 in_0 in_1
	 |   / |
	 HHHHH |
	 / \ | |
	 | HHHHHHH
	 |     |
	 out_0 out_1
	 */
	int data[64] = {
		1, 0, // P table 
		2, 0,
		3, 0,
		4, 0,
		5, 0,
		6, 0,
		7, 0,
		8, 0, // 48
		42, 255-42, // start
		1, 1, // 2in, 2out
		6, 1, // inputs from edge 0,1 (mod 6)
		2, 4, // outputs to edge 2,4
		10, 0, 0, 0, // P table
		0, 10, 0, 0, 
		0, 0, 10, 0, 
		0, 0, 0, 10,
		0, 0, 0, 0, 
		0, 0, 0, 0, 
		0, 0, 0, 0, 
		0, 0, 0, 0,
		42, 255-42, // start
		2, 0, // 3in, 1out
		2, 4, 1, // inputs from edge 2,4,1
		3, // outputs to edge 3
	};
	
	hmm_network hmm(genome(data,data+64), 2, 2, 2);
	BOOST_CHECK(hmm.num_nodes()==2);
	BOOST_CHECK(hmm.num_states()==6);
    
	probabilistic_node* node=dynamic_cast<probabilistic_node*>(hmm.node(0));
	// copied from test_nodes.cpp
	BOOST_CHECK(node->num_inputs()==2);
	BOOST_CHECK(node->num_outputs()==2);	
	BOOST_CHECK(node->xinput(0)==0);
	BOOST_CHECK(node->xoutput(1)==4);
	BOOST_CHECK(node->ptable(0,0)==10);
	BOOST_CHECK(node->ptable(0,1)==1);
	BOOST_CHECK(node->ptable(0,4)==13);	
	BOOST_CHECK(node->ptable(3,3)==10);
    
	node = dynamic_cast<probabilistic_node*>(hmm.node(1));
	// copied from test_nodes.cpp
	BOOST_CHECK(node->num_inputs()==3);
	BOOST_CHECK(node->num_outputs()==1);	
	BOOST_CHECK(node->xinput(0)==2);
	BOOST_CHECK(node->xoutput(0)==3);
	BOOST_CHECK(node->ptable(0,0)==1);
	BOOST_CHECK(node->ptable(0,2)==2);
	BOOST_CHECK(node->ptable(7,0)==8);
	BOOST_CHECK(node->ptable(7,2)==9);
}
