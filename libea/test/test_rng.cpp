#include <boost/test/unit_test.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <ea/rng.h>
#include "test.h"

BOOST_AUTO_TEST_CASE(rng_functional) {
	using namespace ea;
	default_rng_type rng1(1), rng2(1);

	// make sure that they each generate the same bits
	for(int i=0; i<100; ++i) {
		BOOST_CHECK_EQUAL(rng1.bit(), rng2.bit());
	}
}


BOOST_AUTO_TEST_CASE(rng_serialization) {
	using namespace ea;	
	default_rng_type rng1(1), rng2(2);

	std::ostringstream out;
	boost::archive::xml_oarchive oa(out);
	oa << BOOST_SERIALIZATION_NVP(rng1);
	
	std::istringstream in(out.str());
	boost::archive::xml_iarchive ia(in);
	ia >> BOOST_SERIALIZATION_NVP(rng2);

	for(int i=0; i<100; ++i) {
		BOOST_CHECK_EQUAL(rng1.bit(), rng2.bit());
	}	
}
