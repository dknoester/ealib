#include "test.h"

BOOST_AUTO_TEST_CASE(test_md) {
    all_ones_ea ea1, ea2;
	put<POPULATION_SIZE>(10,ea1);
	put<MUTATION_GENOMIC_P>(1.0,ea1);
	
	std::ostringstream out;
	boost::archive::xml_oarchive oa(out);
	oa << BOOST_SERIALIZATION_NVP(ea1);
	
	std::istringstream in(out.str());
	boost::archive::xml_iarchive ia(in);
	ia >> BOOST_SERIALIZATION_NVP(ea2);
	
	BOOST_CHECK_EQUAL(get<POPULATION_SIZE>(ea1), 10u);
	BOOST_CHECK_EQUAL(get<POPULATION_SIZE>(ea1), get<POPULATION_SIZE>(ea2));
	BOOST_CHECK_EQUAL(get<MUTATION_GENOMIC_P>(ea1), 1.0);
	BOOST_CHECK_EQUAL(get<MUTATION_GENOMIC_P>(ea1), get<MUTATION_GENOMIC_P>(ea2));
}
