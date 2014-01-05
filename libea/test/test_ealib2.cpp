#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <ea/representations/bitstring.h>
#include <ea/fitness_functions/all_ones.h>
#include <ea/evolutionary_algorithm.h>
using namespace ealib;


BOOST_AUTO_TEST_CASE(test_ea2) {

    typedef evolutionary_algorithm
    < individual<bitstring, all_ones>
    , ancestors::random_bitstring
    , all_ones
    , mutation::operators::per_site<mutation::site::bit>
    , recombination::asexual
    , generational_models::steady_state< >
    , dont_stop
    , default_callbacks
    > ea_type;

    ea_type ea;
    lifecycle::advance_epoch(10,ea);
}
