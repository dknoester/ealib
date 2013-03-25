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
#ifndef _TEST_LIBEA_H_
#define _TEST_LIBEA_H_

#include <boost/test/unit_test.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <sstream>

#include <ea/evolutionary_algorithm.h>
#include <ea/representations/numeric_vector.h>
#include <ea/fitness_functions/all_ones.h>
#include <ea/checkpoint.h>

using namespace ea;

template <typename EA>
struct configuration : public abstract_configuration<EA> {
    //! Called to generate the initial EA population.
    void initial_population(EA& ea) {
        generate_ancestors(ancestors::random_bitstring(), get<POPULATION_SIZE>(ea), ea);
    }
};

typedef evolutionary_algorithm<
bitstring,
mutation::per_site<mutation::bit>,
all_ones,
configuration
> all_ones_ea;


template <typename EA>
void add_std_meta_data(EA& ea) {
	put<POPULATION_SIZE>(1024,ea);
	put<NUM_OFFSPRING>(2,ea);
	put<REPRESENTATION_SIZE>(10,ea);
	put<MUTATION_GENOMIC_P>(0.1,ea);
	put<MUTATION_PER_SITE_P>(0.1,ea);
	put<TOURNAMENT_SELECTION_N>(2,ea);
	put<TOURNAMENT_SELECTION_K>(1,ea);	
	put<STEADY_STATE_LAMBDA>(1,ea);
	put<ELITISM_N>(2,ea);
	put<REPLACEMENT_RATE_P>(0.5,ea);
}


#endif
