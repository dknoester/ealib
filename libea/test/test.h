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


//#include <ea/representations/numeric_vector.h>
//#include <ea/fitness_functions/all_ones.h>
//#include <ea/interface.h>
//#include <ea/meta_data.h>
//
//using namespace ea;
//
//typedef evolutionary_algorithm<
//	bitstring,
//	all_ones
//	> test_ea;
//
////typedef test_ea::individual<ea::bitstring, ea::all_ones> ti;
//
////struct ti_gen {
////	typedef ti result_type;
////	template <typename EA>
////	ti_gen(EA& ea) : _i(0) { }
////	
////	template <typename Population, typename EA>
////	ti_gen(Population& pop, EA& ea) : _i(0) { }
////	
////	ti_gen() : _i(0) { }
////	template <typename EA>
////	ti operator()(EA& ea) {
////		return operator()();
////	}
////	template <typename Population, typename EA>
////	ti operator()(Population& parents, EA& ea) {
////		return operator()();
////	}
////	
////	template <typename Population, typename EA>
////	void operator()(Population& parents, Population& offspring, std::size_t n, EA& ea) {
////		for( ; n>0; --n) {
////			offspring.insert(offspring.end(), operator()());
////		}
////	}
////	
////	template <typename ParentGenerator, typename OutputIterator, typename EA>
////	void operator()(ParentGenerator& generator, OutputIterator offspring, EA& ea) {
////		*offspring++ = operator()();
////	}	
////	
////	ti operator()() {
////		ea::bitstring ind;
////		ind.resize(8);
////		return ind;
////	}
////	
////	std::size_t _i;
////};
//
//struct ti_mut {
//	template <typename Representation, typename MetaData, typename EA>
//	void operator()(Representation& repr, MetaData& md, EA& ea) {
//	}
//};		
//
//
////struct test_evolutionary_algorithm {
////	typedef double fitness_type;
////	typedef ea::all_ones fitness_function_type;
////	typedef ti individual_type;
////	typedef ea::default_rng_type rng_type;
////	typedef ti_gen individual_generator_type;
////	typedef ea::bitstring representation_type;
////	typedef ti_gen parent_selection_type;
////	typedef ti_gen recombination_operator_type;
////	typedef ti_gen survivor_selection_type;
////	typedef ti_mut mutation_operator_type;
////	
////	ea::default_rng_type& rng() { return _rng; }
////	ea::all_ones& fitness_function() { return _fcn; }
////	
////	ea::default_rng_type _rng;
////	ea::all_ones _fcn;
////};
//
////typedef ea::population<ti> test_pop;
//
////typedef test_evolutionary_algorithm test_ea;
//
#endif
