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
    test_generational_model<ea::generational_models::death_birth_process< > >();
}
