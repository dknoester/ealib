/* test_recombination.cpp
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
#include "test_libea.h"

#include <ea/recombination.h>


//! Helper function to test recombination operators.
template <typename RecombinationOperator>
void test_recombination() {
//    test_ea ea;
//    put<MUTATION_GENOMIC_P>(1.0,ea);
//    put<REPRESENTATION_SIZE>(10,ea);
    
    // create the parent and offspring populations:
//    test_ea::population_type parents, offspring;
//    generate_individuals_n(parents, 2, ea);
//    
//    typename test_ea::parent_selection_type parent_selector;
//    RecombinationOperator r;
//    recombine(parents, offspring, r, ea);
//    BOOST_CHECK(parents.size()==2);
//    BOOST_CHECK(offspring.size()==RecombinationOperator::parents_required);
}


/*! Recombination operator unit tests.
 */
BOOST_AUTO_TEST_CASE(recombination_functional) {
	test_recombination<ea::recombination::single_point_crossover>();
	test_recombination<ea::recombination::two_point_crossover>();
	test_recombination<ea::recombination::asexual>();
}
