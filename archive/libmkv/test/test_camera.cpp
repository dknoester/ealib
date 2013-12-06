/* test_camera.cpp
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
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <mkv/markov_network.h>
#include <mkv/camera.h>
#include "test.h"

/*! Test for a 2D camera iterator over a matrix.
 */
BOOST_AUTO_TEST_CASE(test_camera2) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace ealib;
    using namespace boost::numeric::ublas;
    
    typedef matrix<int> Matrix;
    
    Matrix M(4,4);
    int k=0;
    for(std::size_t i=0; i<M.size1(); ++i) {
        for(std::size_t j=0; j<M.size2(); ++j) {
            M(i,j) = k++;
        }
    }
    
    camera2_iterator<Matrix> ci(M,2,2);
    BOOST_CHECK_EQUAL(ci[0], 0);
    BOOST_CHECK_EQUAL(ci[1], 1);
    BOOST_CHECK_EQUAL(ci[2], 4);
    BOOST_CHECK_EQUAL(ci[3], 5);
    
    ci.move(1,1);
    BOOST_CHECK_EQUAL(ci[0], 5);
    BOOST_CHECK_EQUAL(ci[1], 6);
    BOOST_CHECK_EQUAL(ci[2], 9);
    BOOST_CHECK_EQUAL(ci[3], 10);

    ci.move(1,1);
    BOOST_CHECK_EQUAL(ci[0], 10);
    BOOST_CHECK_EQUAL(ci[1], 11);
    BOOST_CHECK_EQUAL(ci[2], 14);
    BOOST_CHECK_EQUAL(ci[3], 15);

    ci.position(3,3); // this clips to the image boundary
    BOOST_CHECK_EQUAL(ci[0], 10);
    BOOST_CHECK_EQUAL(ci[1], 11);
    BOOST_CHECK_EQUAL(ci[2], 14);
    BOOST_CHECK_EQUAL(ci[3], 15);
    
    ci.move(-1,0);
    BOOST_CHECK_EQUAL(ci[0], 6);
    BOOST_CHECK_EQUAL(ci[1], 7);
    BOOST_CHECK_EQUAL(ci[2], 10);
    BOOST_CHECK_EQUAL(ci[3], 11);
}

/*! Test for a 3D camera iterator over a matrix.
 */
BOOST_AUTO_TEST_CASE(test_camera3) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace ealib;
    using namespace boost::numeric::ublas;
    
    typedef matrix<int> Matrix;
    
    Matrix M(4,4);
    int k=0;
    for(std::size_t i=0; i<M.size1(); ++i) {
        for(std::size_t j=0; j<M.size2(); ++j) {
            M(i,j) = k++;
        }
    }
    
    camera3_iterator<Matrix> ci(M,2,2);
    BOOST_CHECK_EQUAL(ci[0], 0);
    BOOST_CHECK_EQUAL(ci[1], 1);
    BOOST_CHECK_EQUAL(ci[2], 4);
    BOOST_CHECK_EQUAL(ci[3], 5);
    
    ci.move_ij(1,1);
    BOOST_CHECK_EQUAL(ci[0], 5);
    BOOST_CHECK_EQUAL(ci[1], 6);
    BOOST_CHECK_EQUAL(ci[2], 9);
    BOOST_CHECK_EQUAL(ci[3], 10);
    
    ci.move_xy(1,1);
    BOOST_CHECK_EQUAL(ci[0], 10);
    BOOST_CHECK_EQUAL(ci[1], 11);
    BOOST_CHECK_EQUAL(ci[2], 14);
    BOOST_CHECK_EQUAL(ci[3], 15);
    
    ci.move(mkv::Z_AXIS, 1);
    BOOST_CHECK_EQUAL(ci[0], 15);
    BOOST_CHECK_EQUAL(ci[1], 0);
    BOOST_CHECK_EQUAL(ci[2], 0);
    BOOST_CHECK_EQUAL(ci[3], 0);
    
    ci.move_ij(-2,-2);
    BOOST_CHECK_EQUAL(ci[0], 5);
    BOOST_CHECK_EQUAL(ci[1], 7);
    BOOST_CHECK_EQUAL(ci[2], 13);
    BOOST_CHECK_EQUAL(ci[3], 15);
    
    ci.move_ij(-3,-3);
    BOOST_CHECK_EQUAL(ci[0], 0);
    BOOST_CHECK_EQUAL(ci[1], 0);
    BOOST_CHECK_EQUAL(ci[2], 0);
    BOOST_CHECK_EQUAL(ci[3], 0);
    
    ci.home();
    ci.move_ij(1,0);
    BOOST_CHECK_EQUAL(ci[0], 4);
    BOOST_CHECK_EQUAL(ci[1], 5);
    BOOST_CHECK_EQUAL(ci[2], 8);
    BOOST_CHECK_EQUAL(ci[3], 9);
}

