/* test_markov_network.cpp
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester.
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
#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <ea/mkv/markov_network_evolution.h>
#include <ea/data_structures/circular_vector.h>



BOOST_AUTO_TEST_CASE(test_logic_gate) {
    using namespace ealib;
    using namespace mkv;
    
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
    //	 /
    //	 2-out 3-out
    
	int data[] = {
		42, 255-42, // start
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
    circular_vector<int> genome(data, data+32);

    markov_network< > N(2,2,1);
    translators::markov_network_translator translate(1,8,1,8);
    translate.translate_genome(N,genome);
    
    BOOST_CHECK(N.ngates()==1);
    BOOST_CHECK(N.nstates()==5);
    
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
	in=tc0;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
    
	in=tc1;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
    
	in=tc2;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
    
	in=tc3;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
}

BOOST_AUTO_TEST_CASE(test_probabilistic_gate) {
    using namespace ealib;
    using namespace mkv;
    
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
    //	 /
    //	 2-out 3-out
    
	int data[255] = {
		43, 255-43, // start
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
    circular_vector<int> genome(data, data+32);
    
    markov_network< > N(2,2,1);
    translators::markov_network_translator translate(1,8,1,8);
    translate.translate_genome(N,genome);
    
    BOOST_CHECK(N.ngates()==1);
    BOOST_CHECK(N.nstates()==5);
    BOOST_CHECK(N.ninputs()==2);
    BOOST_CHECK(N.noutputs()==2);
    BOOST_CHECK(N.nhidden()==1);
    
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
    
    in=tc0;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
    
	in=tc1;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
    
	in=tc2;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
    
	in=tc3;
    N.update(in);
    BOOST_CHECK(std::equal(N.begin_output(), N.end_output(), &in[2]));
}

BOOST_AUTO_TEST_CASE(test_markov_network) {
    using namespace ealib;
    using namespace mkv;
    
    int data[] = {
        5, 0,
        6, 0,
        7, 0,
        8, 0, // 64
        42, 255-42, // start
        1, 1, // 2in, 2out
        0, 1, // inputs from edge 0,1
        2, 4, // outputs to edge 2,4
        2, // D table (these are column indices)
        1,
        3,
        0, // 76
        43, 255-43, // start
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
        43, 255-43, // start
        2, 0, // 3in, 1out
        2, 4, 1, // inputs from edge 2,4,1
        3, // outputs to edge 3 // 48
        1, 0, // P table
        2, 0,
        3, 0,
        4, 0
    };
    circular_vector<int> genome(data, data+76);

    markov_network< > N(2,2,2,42);
    translators::markov_network_translator translate(1,8,1,8);
    translate.translate_genome(N,genome);

    BOOST_CHECK(N.ngates()==3);
    BOOST_CHECK(N.nstates()==6);

    {
        typedef logic_gate<default_rng_type> gate_type;
        gate_type& g = *static_cast<gate_type*>(N.gates()[0].get());
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
        typedef probabilistic_gate<default_rng_type> gate_type;
        gate_type& g = *static_cast<gate_type*>(N.gates()[1].get());
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
        typedef probabilistic_gate<default_rng_type> gate_type;
        gate_type& g = *static_cast<gate_type*>(N.gates()[2].get());
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
