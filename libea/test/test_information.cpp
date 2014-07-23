/* test_information.cpp
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
#include <boost/graph/adjacency_list.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <iterator>
#include <algorithm>
#include "test.h"
#include <ea/math/information.h>


/*! Tests of entropy.
 */
BOOST_AUTO_TEST_CASE(entropy_functional) {
    using namespace ealib::math;
    using namespace boost::numeric::ublas;
    typedef matrix<unsigned int> Matrix;
    typedef matrix_column<Matrix> Column;
    Matrix m(4,2);
    Column x(m,0);
    x(0)=0; x(1)=0; x(2)=1; x(3)=1;
    Column y(m,1);
    y(0)=0; y(1)=1; y(2)=0; y(3)=1;
    
    BOOST_CHECK_EQUAL(entropy(x), 1.0);
    BOOST_CHECK_EQUAL(entropy(y), 1.0);
    BOOST_CHECK_EQUAL(joint_entropy(m), 2.0);
}

/*! Tests of information.
 */
BOOST_AUTO_TEST_CASE(information_functional) {
    using namespace ealib::math;
    using namespace boost::numeric::ublas;
    typedef matrix<unsigned int> Matrix;
    typedef matrix_column<Matrix> Column;
    Matrix m(4,2);
    Column x(m,0);
    x(0)=0; x(1)=0; x(2)=1; x(3)=1;
    Column y(m,1);
    y(0)=0; y(1)=1; y(2)=0; y(3)=1;

    BOOST_CHECK_EQUAL(mutual_information(x,y), 0.0);
    BOOST_CHECK_EQUAL(joint_mutual_information(m,y), 1.0);
    
    y = x;
    BOOST_CHECK_EQUAL(mutual_information(x,y),1.0);
}


/*! Probability mass function tests.
 */
BOOST_AUTO_TEST_CASE(pmf_functional) {
    using namespace ealib::math;
    
    unsigned int x[]={0,0,1,1};
    pmf<unsigned int> p = probability_mass_function(x,&x[4]);
    
    BOOST_CHECK_EQUAL(p.event_count(), 4u);
    BOOST_CHECK_EQUAL(p.size(), 2);
    BOOST_CHECK_EQUAL(p[0], 0.5);
    BOOST_CHECK_EQUAL(p[1], 0.5);
    
    unsigned int y[]={0,0,0,0,1,1,1,2,2,3};
    p = probability_mass_function(y,&y[10]);
    BOOST_CHECK_EQUAL(p.event_count(), 10);
    BOOST_CHECK_EQUAL(p.size(), 4);
    BOOST_CHECK_EQUAL(p[0], 0.4);
    BOOST_CHECK_EQUAL(p[1], 0.3);
    BOOST_CHECK_EQUAL(p[2], 0.2);
    BOOST_CHECK_EQUAL(p[3], 0.1);
    
    std::vector<unsigned int> x1;
    x1.push_back(0);    x1.push_back(0);    x1.push_back(1);    x1.push_back(1);
    p = probability_mass_function(x1.begin(), x1.end());
    BOOST_CHECK_EQUAL(p.event_count(), 4);
    BOOST_CHECK_EQUAL(p.size(), 2);
    BOOST_CHECK_EQUAL(p[0], 0.5);
    BOOST_CHECK_EQUAL(p[1], 0.5);
}
