/* test_generational.cpp
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
#include "test.h"
#include <ea/initialization.h>
#include <ea/generational_models/death_birth_process.h>
#include <ea/generational_models/synchronous.h>


//! Helper function to test generational models.
template <typename GenerationalModel>
void test_generational_model() {
	using namespace ea;
    
    typedef evolutionary_algorithm<
    bitstring,
    mutation::per_site<mutation::bit>,
    all_ones,
    recombination::asexual,
	GenerationalModel> test_ea;
    
	try {
		test_ea ea;
		add_std_meta_data(ea);
		put<POPULATION_SIZE>(10,ea);
		
        ea.initialize();
        generate_initial_population(ea);        
        ea.update();

		BOOST_CHECK(ea.population().size()==10);
        BOOST_CHECK(ea.current_update()==1ul);
	} catch(ealib_exception& ex) {
		throw ex.msg;
	}	
}


/*! Recombination operator unit tests.
 */
BOOST_AUTO_TEST_CASE(generational_functional) {
	test_generational_model<ea::generational_models::synchronous< > >();
    test_generational_model<ea::generational_models::death_birth_process>();
}
