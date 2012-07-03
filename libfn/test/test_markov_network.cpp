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
#include <fn/markov_network.h>

#include "test.h"

/*!
 */
BOOST_AUTO_TEST_CASE(test_probabilistic_mkv_node_ctor) {
    using namespace fn;
	using namespace fn::detail;
    
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
    probabilistic_mkv_node node(inputs, outputs, &data[8]);
    
	BOOST_CHECK(node._table(0,0)==10);
	BOOST_CHECK(node._table(0,1)==0);
	BOOST_CHECK(node._table(1,1)==10);
	BOOST_CHECK(node._table(3,2)==0);
	BOOST_CHECK(node._table(3,3)==10);
    BOOST_CHECK(node._table(2,4)==10); // row sum
}

/*!
 */
BOOST_AUTO_TEST_CASE(test_synprob_mkv_node_ctor) {
    using namespace fn;
	using namespace fn::detail;
    
	int data[48]={
		42, 255-42, // start
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
    
    synprob_mkv_node node(data[4],
                          data[5], poswv,
                          data[6], negwv,
                          inputs, outputs, &data[17]);
    
	BOOST_CHECK(node._table(0,0)==10);
	BOOST_CHECK(node._table(0,1)==0);
	BOOST_CHECK(node._table(1,1)==10);
	BOOST_CHECK(node._table(3,2)==0);
	BOOST_CHECK(node._table(3,3)==10);
    BOOST_CHECK(node._table(2,4)==10); // row sum
}

/*!
 */
BOOST_AUTO_TEST_CASE(test_deterministic_mkv_node_ctor) {
    using namespace fn;
	using namespace fn::detail;
    
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
    
	BOOST_CHECK(node._table(0,0)==2);
	BOOST_CHECK(node._table(1,0)==1);
	BOOST_CHECK(node._table(2,0)==3);
	BOOST_CHECK(node._table(3,0)==0);
}

/*! 
 */
BOOST_AUTO_TEST_CASE(test_markov_network_ctor) {
    using namespace fn;
	using namespace fn::detail;
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
    put<NODE_INPUT_FLOOR>(1, mkv);
    put<NODE_INPUT_LIMIT>(8, mkv);
    put<NODE_OUTPUT_FLOOR>(1, mkv);
    put<NODE_OUTPUT_LIMIT>(8, mkv);
    put<NODE_HISTORY_FLOOR>(1, mkv);
    put<NODE_HISTORY_LIMIT>(8, mkv);
    
    build_markov_network(mkv, data, data+105);
    BOOST_CHECK(mkv.size()==4);
    
    {
        probabilistic_mkv_node& node = *dynamic_cast<probabilistic_mkv_node*>(mkv.begin()->get());
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==0);
        BOOST_CHECK(node._table(2,4)==10);
    }
    {
        probabilistic_mkv_node& node = *dynamic_cast<probabilistic_mkv_node*>((mkv.begin()+1)->get());
        BOOST_CHECK(node._table(0,0)==1);
        BOOST_CHECK(node._table(1,0)==2);
        BOOST_CHECK(node._table(7,0)==8);
    }
    {
        deterministic_mkv_node& node = *dynamic_cast<deterministic_mkv_node*>((mkv.begin()+2)->get());
        BOOST_CHECK(node._table(0,0)==2);
        BOOST_CHECK(node._table(1,0)==1);
        BOOST_CHECK(node._table(2,0)==3);
    }
    {
        synprob_mkv_node& node = *dynamic_cast<synprob_mkv_node*>((mkv.begin()+3)->get());
        BOOST_CHECK(node._table(0,0)==10);
        BOOST_CHECK(node._table(0,1)==0);
        BOOST_CHECK(node._table(2,4)==10);
    }
    
}
