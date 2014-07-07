/* test_neuroevolution.cpp
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

#include <ea/evolutionary_algorithm.h>
#include <ea/generational_models/moran_process.h>
#include <ea/genome_types/realstring.h>
#include <ea/fitness_functions/pole_balancing.h>
#include <ea/ann/neuroevolution.h>
using namespace ealib;

BOOST_AUTO_TEST_CASE(test_basic_direct) {
	typedef evolutionary_algorithm
	< direct<ann::basic_neural_network< > >
	, pole_balancing
	, mutation::operators::weight_matrix
	, recombination::asexual
	, generational_models::moran_process< >
	, ancestors::random_weight_neural_network
	> ea_type;
	
    ea_type ea;
}
