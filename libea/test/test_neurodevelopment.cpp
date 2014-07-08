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
#include <boost/test/unit_test.hpp>

#include <ea/evolutionary_algorithm.h>
#include <ea/generational_models/moran_process.h>
#include <ea/fitness_functions/pole_balancing.h>
#include <ea/fitness_functions/quiet_nan.h>
#include <ea/ann/neurodevelopment.h>
#include <ann/basic_neural_network.h>
using namespace ealib;

/* This test checks for producing an ANN from a developmental template via the
 phi translator.
 */
BOOST_AUTO_TEST_CASE(test_phi) {
	typedef evolutionary_algorithm
	< indirect<graph::developmental_graph, ann::basic_neural_network< >, translators::phi>
	, pole_balancing
	, mutation::operators::delta_growth
	, recombination::asexual
	, generational_models::moran_process< >
	, ancestors::random_delta_growth_graph
	> ea_type;

	ea_type ea;
}

/* This checks for directly evolving a delta graph (which isn't a graph, but is
 the parameters needed to build a graph).
 */
BOOST_AUTO_TEST_CASE(test_delta_direct) {
	typedef evolutionary_algorithm
	< direct<graph::delta_graph>
	, quiet_nan
	, mutation::operators::delta
	, recombination::asexual
	, generational_models::moran_process< >
	, ancestors::random_delta_graph
	> ea_type;
	
	ea_type ea;
}

/* And here we're evolving the delta graph, but using an ANN as the phenotype.
 */
BOOST_AUTO_TEST_CASE(test_delta_indirect) {
	typedef evolutionary_algorithm
	< indirect<graph::delta_graph, ann::basic_neural_network< >, translators::delta>
	, pole_balancing
	, mutation::operators::delta
	, recombination::asexual
	, generational_models::moran_process< >
	, ancestors::random_delta_graph
	> ea_type;
	
	ea_type ea;
}
