/* test_graph.cpp
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
#include <ea/graph.h>
#include "test.h"

struct graph_fitness : public fitness_function<unary_fitness<double> > {
    template <typename Individual, typename EA>
    double operator()(Individual& ind, EA& ea) {
//        typename EA::representation_type& G=ind.repr();
        return 1.0;
    }
};


typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, graph::mutable_vertex, graph::mutable_edge> Graph;

typedef evolutionary_algorithm<
Graph,
ancestors::random_graph,
mutation::graph_mutator,
graph_fitness,
abstract_configuration,
recombination::asexual
> graph_ea;

BOOST_AUTO_TEST_CASE(test_graph_mutations) {
	using namespace ealib;
    namespace eg = ealib::graph;
    
    graph_ea ea;
    Graph G;
    
    eg::add_vertex(G,ea.rng());
    eg::add_vertex(G,ea.rng());
    BOOST_CHECK_EQUAL(num_vertices(G), 2);
    
    eg::add_edge(G,ea.rng());
    BOOST_CHECK_EQUAL(degree(vertex(0,G),G), 1);
    eg::remove_edge(G,ea.rng());
    BOOST_CHECK_EQUAL(degree(vertex(0,G),G), 0);
    eg::add_edge(G,ea.rng());
    
    eg::duplicate_vertex(G,ea.rng());
    BOOST_CHECK_EQUAL(num_vertices(G), 3);
    BOOST_CHECK_EQUAL(degree(vertex(2,G),G), 1);
    BOOST_CHECK((degree(vertex(0,G),G)==2) || (degree(vertex(1,G),G)==2));

    eg::merge_vertices(G,ea.rng());
    BOOST_CHECK_EQUAL(num_vertices(G), 2);
    BOOST_CHECK((degree(vertex(0,G),G)>=1) && (degree(vertex(1,G),G)>=1));
    
    eg::remove_vertex(G,ea.rng());
    BOOST_CHECK_EQUAL(num_vertices(G), 1);
    BOOST_CHECK((degree(vertex(0,G),G) == 0) || (degree(vertex(0,G),G) == 2)); // 2 is for self-loops as a result of a merge
}


BOOST_AUTO_TEST_CASE(test_graph_growth) {
	using namespace ealib;
    using namespace ealib::graph;
    growth_descriptor desc;
    desc.Pc[conditional::p] = 0.5;
    desc.Pc[conditional::q] = 0.8;
    desc.Pc[conditional::r] = 0.75;
    default_rng_type rng(1);
    Graph G;
    graph::grow_network(G, 100, desc, rng);
}