#include "test.h"

#include <ea/selection/fitness_proportional.h>
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
