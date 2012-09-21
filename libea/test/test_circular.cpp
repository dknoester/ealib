/* test_circular.cpp
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

#include <ea/cvector.h>
#include "test_libea.h"

BOOST_AUTO_TEST_CASE(test_circular_vector) {
	using namespace ea;

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

    typedef cvector<int> cv_type;
    cv_type cv(data, data+12);
    BOOST_CHECK(cv.size()==12);
    BOOST_CHECK(*cv.rbegin()==32767);
    
    cv_type::iterator i=cv.begin();
    BOOST_CHECK(*i==44);
    
    std::size_t c=0;
    for( ; i!=cv.end(); ++i, ++c) { }
    BOOST_CHECK(c==12);
    BOOST_CHECK(*i==44);

    std::advance(i, cv.size()-1);
    BOOST_CHECK(*i==32767);
    BOOST_CHECK(*++i==44);
    ++i;
    std::advance(i, 3*cv.size());
    BOOST_CHECK((*i)==(255-44));
}
