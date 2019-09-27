/* test_rng.cpp
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
#include <boost/test/unit_test.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <ea/rng.h>
#include "test.h"

BOOST_AUTO_TEST_CASE(rng_functional) {
	using namespace ealib;
	default_rng_type rng1(1), rng2(1);

	// make sure that they each generate the same bits
	for(int i=0; i<100; ++i) {
		BOOST_CHECK_EQUAL(rng1.bit(), rng2.bit());
	}
    
    default_rng_type rng3(rng1);
	for(int i=0; i<100; ++i) {
		BOOST_CHECK_EQUAL(rng1.bit(), rng3.bit());
	}    
    
    rng2 = rng3;
	for(int i=0; i<100; ++i) {
		BOOST_CHECK_EQUAL(rng2.bit(), rng3.bit());
	}    
}


BOOST_AUTO_TEST_CASE(rng_serialization) {
	using namespace ealib;	
	default_rng_type rng1(1), rng2(2);

	std::ostringstream out;
    { // archives have to go out-of-scope to flush contents: https://github.com/boostorg/serialization/issues/82
        boost::archive::xml_oarchive oa(out);
        oa << BOOST_SERIALIZATION_NVP(rng1);
    }
	
	std::istringstream in(out.str());
	boost::archive::xml_iarchive ia(in);
	ia >> BOOST_SERIALIZATION_NVP(rng2);

	for(int i=0; i<100; ++i) {
		BOOST_CHECK_EQUAL(rng1.bit(), rng2.bit());
	}	
}
