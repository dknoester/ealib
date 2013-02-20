/* test_graph.cpp
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
#include <ea/graph.h>
#include "test.h"

struct graph_fitness : public fitness_function<unary_fitness<double> > {
    template <typename Individual, typename EA>
    double operator()(Individual& ind, EA& ea) {
        typename EA::representation_type& G=ind.repr();
        return 1.0;
    }
};

template <typename EA>
struct graph_configuration : public abstract_configuration<EA> {
    void initial_population(EA& ea) {
        generate_ancestors(ancestors::random_graph(), get<POPULATION_SIZE>(ea), ea);
    }
};

typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, abstract_vertex< >, abstract_edge< > > Graph;

typedef evolutionary_algorithm<
Graph,
mutation::graph_mutation,
graph_fitness,
graph_configuration
> graph_ea;

BOOST_AUTO_TEST_CASE(test_graph_mutations) {
	using namespace ea;
    namespace emd = ea::mutation::detail;
    
    graph_ea ea;
    Graph G;
    
    emd::add_vertex(G,ea);
    emd::add_vertex(G,ea);
    BOOST_CHECK_EQUAL(num_vertices(G), 2);
    
    emd::add_edge(G,ea);
    BOOST_CHECK_EQUAL(degree(vertex(0,G),G), 1);
    emd::remove_edge(G,ea);
    BOOST_CHECK_EQUAL(degree(vertex(0,G),G), 0);
    emd::add_edge(G,ea);
    
    emd::duplicate_vertex(G,ea);
    BOOST_CHECK_EQUAL(num_vertices(G), 3);
    BOOST_CHECK_EQUAL(degree(vertex(2,G),G), 1);
    BOOST_CHECK((degree(vertex(0,G),G)==2) || (degree(vertex(1,G),G)==2));

    emd::merge_vertices(G,ea);
    BOOST_CHECK_EQUAL(num_vertices(G), 2);
    BOOST_CHECK((degree(vertex(0,G),G)>=1) && (degree(vertex(1,G),G)>=1));
    
    emd::remove_vertex(G,ea);
    BOOST_CHECK_EQUAL(num_vertices(G), 1);
    BOOST_CHECK((degree(vertex(0,G),G) == 0) || (degree(vertex(0,G),G) == 2)); // 2 is for self-loops as a result of a merge
}


