/* test_neural_network.cpp
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

#include <ann/feed_forward.h>

BOOST_AUTO_TEST_CASE(test_logistic) {
    using namespace ealib;
    using namespace ann;
    feed_forward< > N(2);
    N.link(0,1) = 1.0;
    
    N[0] = 1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.99, 1.0);

    N[0] = 0.5;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.95, 1.0);

    N[0] = 0.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.5, 1.0);

    N[0] = -1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.00247, 1.0);
}

BOOST_AUTO_TEST_CASE(test_heaviside) {
    using namespace ealib;
    using namespace ann;
    feed_forward<heaviside> N(2);
    N.link(0,1) = 1.0;
    
    N[0] = 1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 1.0, 1.0);
    
    N[0] = 0.5;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 1.0, 1.0);
    
    N[0] = 0.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.0, 1.0);
    
    N[0] = -1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.0, 1.0);
}

BOOST_AUTO_TEST_CASE(test_htan) {
    using namespace ealib;
    using namespace ann;
    feed_forward<hyperbolic_tangent> N(2);
    N.link(0,1) = 1.0;
    
    N[0] = 1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.99, 1.0);
    
    N[0] = 0.5;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.9, 1.0);
    
    N[0] = 0.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.0, 1.0);
    
    N[0] = -1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], -0.99, 1.0);
}

BOOST_AUTO_TEST_CASE(test_clipping_htan) {
    using namespace ealib;
    using namespace ann;
    feed_forward<hyperbolic_tangent,clip<double> > N(2, clip<double>(-0.95,-1.0,0.95,1.0));
    N.link(0,1) = 1.0;
    
    N[0] = 1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 1.0, 1.0);
    
    N[0] = 0.5;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.9, 1.0);
    
    N[0] = 0.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], 0.0, 1.0);
    
    N[0] = -0.5;
    N.update();
    BOOST_CHECK_CLOSE(N[1], -0.90, 1.0);
    
    N[0] = -1.0;
    N.update();
    BOOST_CHECK_CLOSE(N[1], -1.0, 1.0);
}
