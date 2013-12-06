/* test.h
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
#ifndef _TEST_H_
#define _TEST_H_

#include <boost/test/unit_test.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <sstream>

#include <ea/analysis.h>
#include <ea/lifecycle.h>
#include <ea/evolutionary_algorithm.h>
#include <ea/representations/bitstring.h>
#include <ea/fitness_functions/all_ones.h>

using namespace ealib;

template <typename EA>
struct configuration : public abstract_configuration<EA> {
    //! Called to generate the initial EA population.
    void initial_population(EA& ea) {
        generate_ancestors(ancestors::random_bitstring(), get<POPULATION_SIZE>(ea), ea);
    }
    
    //! Called to generate n individuals into the given population.
    void fill_population(EA& ea) {
        generate_ancestors(ancestors::random_bitstring(), get<POPULATION_SIZE>(ea)-ea.size(), ea);
    }
};

typedef evolutionary_algorithm<
bitstring,
mutation::operators::per_site<mutation::site::bit>,
all_ones,
configuration
> all_ones_ea;

typedef evolutionary_algorithm<
bitstring,
mutation::operators::per_site<mutation::site::bit>,
multi_all_ones,
configuration
> multi_all_ones_ea;


template <typename EA>
void add_std_meta_data(EA& ea) {
	put<POPULATION_SIZE>(1024,ea);
	put<STEADY_STATE_LAMBDA>(2,ea);
	put<REPRESENTATION_SIZE>(10,ea);
	put<MUTATION_GENOMIC_P>(0.1,ea);
	put<MUTATION_PER_SITE_P>(0.1,ea);
	put<TOURNAMENT_SELECTION_N>(2,ea);
	put<TOURNAMENT_SELECTION_K>(1,ea);	
}


#endif
