/* test_mkv.cpp
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
#include <mkv/convolution.h>
#include <mkv/markov_network.h>
#include "test.h"

/*! Test for convolving Markov networks.
 */
BOOST_AUTO_TEST_CASE(test_convolution) {
    using namespace mkv;
	using namespace mkv::detail;
    using namespace ealib;
    using namespace boost::numeric::ublas;

    typedef matrix<double> Matrix;
    
    std::size_t n=8, m=10;
    Matrix M(n,m), P;
    for(std::size_t i=0; i<M.size1(); ++i) {
        for(std::size_t j=0; j<M.size2(); ++j) {
            M(i,j) = i*M.size2() + j;
        }
    }
    
    convolve_data2d(M, 3, 3, max_pool(), P);    
    BOOST_CHECK_EQUAL(P.size1(), 6);
    BOOST_CHECK_EQUAL(P.size2(), 8);
    BOOST_CHECK_EQUAL(P(0,0), 22);
    BOOST_CHECK_EQUAL(P(0,1), 23);
    BOOST_CHECK_EQUAL(P(1,0), 32);
    
    convolve_data2d(M, 2, 2, max_pool(), P, 2, 2);
    BOOST_CHECK_EQUAL(P.size1(), 3);
    BOOST_CHECK_EQUAL(P.size2(), 4);
    BOOST_CHECK_EQUAL(P(0,0), 11);
    BOOST_CHECK_EQUAL(P(0,1), 13);
}
