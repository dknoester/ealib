/* test_deep_markov_network.cpp
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

#include <ea/mkv/deep_markov_network.h>
#include <ea/cvector.h>
#include <ea/translation.h>

/*! Test for deep Markov Networks.
 */
BOOST_AUTO_TEST_CASE(test_deep_markov_network) {
    using namespace ealib;
    using namespace ealib::mkv;
    
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
    
	int data[100] = {
		42, 255-42, // start
        0, // layer 0
        3, 1, // 4in, 2out
		0, 1, 2, 3, // inputs from state 0-3
		4, 5, // outputs to state 4-5
		0, // d table; echos input
		1,
		2,
		3,
		0, // d table; echos input; 16
		1,
		2,
		3,
		0, // d table; echos input
		1,
		2,
		3,
		0, // d table; echos input
		1,
		2,
		3,
        0, // 28
		42, 255-42, // start ; 30
        1, // layer 0
        1, 3, // 2in, 4out
		0, 1, // inputs from state 0-1
		2, 3, 4, 5, // outputs to state 2-5
		1, // d table; echos 2^in
		2,
		4,
		8,
        0 // 44
    };
    ealib::cvector<int> genome(data, data+44);
    desc_type layers[2] = { desc_type(4,2,0), desc_type(2,4,0) };
    
    deep_markov_network< > N(layers, layers+2, 42);
    ealib::translate_genome(genome,
                            start_codon(),
                            deep_genome_translator(1, 8, 1, 8),
                            N);
    
    BOOST_CHECK(N.nlayers()==2);
    BOOST_CHECK(N[0].nstates()==6);
    BOOST_CHECK(N[0].ngates()==1);
    BOOST_CHECK(N[1].nstates()==6);
    BOOST_CHECK(N[1].ngates()==1);
    
    {
        typedef logic_gate<ealib::default_rng_type> gate_type;
        gate_type& g = *static_cast<gate_type*>(N[0].gates()[0].get());
        BOOST_CHECK(g.inputs[0]==0);
        BOOST_CHECK(g.inputs[1]==1);
        BOOST_CHECK(g.inputs[2]==2);
        BOOST_CHECK(g.inputs[3]==3);
        BOOST_CHECK(g.outputs[0]==4);
        BOOST_CHECK(g.outputs[1]==5);
        BOOST_CHECK(g.M[0]==0);
        BOOST_CHECK(g.M[1]==1);
        BOOST_CHECK(g.M[2]==2);
        BOOST_CHECK(g.M[3]==3);
    }
    {
        typedef logic_gate<ealib::default_rng_type> gate_type;
        gate_type& g = *static_cast<gate_type*>(N[1].gates()[0].get());
        BOOST_CHECK(g.inputs[0]==0);
        BOOST_CHECK(g.inputs[1]==1);
        BOOST_CHECK(g.outputs[0]==2);
        BOOST_CHECK(g.outputs[1]==3);
        BOOST_CHECK(g.outputs[2]==4);
        BOOST_CHECK(g.outputs[3]==5);
        BOOST_CHECK(g.M[0]==1);
        BOOST_CHECK(g.M[1]==2);
        BOOST_CHECK(g.M[2]==4);
        BOOST_CHECK(g.M[3]==8);
    }


    // test cases:
	int tc0[8] = {
		0, 0, 0, 0, // inputs
		1, 0, 0, 0 // expected outputs
	};
	int tc1[8] = {
		1, 0, 0, 0,
		0, 1, 0, 0
	};
	int tc2[8] = {
		0, 1, 0, 0,
		0, 0, 1, 0
	};
	int tc3[8] = {
		1, 1, 0, 0,
		0, 0, 0, 1
	};

    int* in;
    int inputs[2] = { 4, 2 };
    
    in=tc0;
    N.cascade_update(in);
    BOOST_CHECK(std::equal(N[1].begin_output(), N[1].end_output(), &in[4]));

	in=tc1;
    N.cascade_update(in);
    BOOST_CHECK(std::equal(N[1].begin_output(), N[1].end_output(), &in[4]));

	in=tc2;
    N.cascade_update(in);
    BOOST_CHECK(std::equal(N[1].begin_output(), N[1].end_output(), &in[4]));

	in=tc3;
    N.cascade_update(in);
    BOOST_CHECK(std::equal(N[1].begin_output(), N[1].end_output(), &in[4]));
}
