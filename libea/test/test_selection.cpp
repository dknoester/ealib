#include "test.h"

#include <ea/interface.h>
#include <ea/selection/truncation.h>
#include <ea/selection/elitism.h>
#include <ea/selection/proportional.h>
#include <ea/selection/random.h>
#include <ea/selection/tournament.h>


//! Helper function to test selection strategies.
template <typename SelectionStrategy>
void test_selection_strategy() {
	using namespace ea;
	try {
//		test_ea ea;
//		add_std_meta_data(ea);
		
//		// create two populations:
//		test_ea::population_type p, d;
//		generate_individuals_n(p, 10, ea);
//		BOOST_CHECK(p.size()==10);
//		BOOST_CHECK(d.empty());
//		
//		// and select from the source into the destination:
//		SelectionStrategy selector;
//		select_n(p, d, selector, 5, ea);
//		BOOST_CHECK(d.size()==5);
	} catch(ealib_exception& ex) {
		throw ex.msg;
	}	
}


/*! Selection strategy unit test.
 */
BOOST_AUTO_TEST_CASE(selection_functional) {
	test_selection_strategy<ea::selection::truncation>();
	test_selection_strategy<ea::selection::proportional< > >();
	test_selection_strategy<ea::selection::random>();
	test_selection_strategy<ea::selection::tournament< > >();
	test_selection_strategy<ea::selection::elitism<ea::selection::tournament< > > >();
}
