#include "test.h"

/*! Test of EA checkpointing.
 */
BOOST_AUTO_TEST_CASE(ealib_checkpoint) {
    all_ones_ea ea1, ea2;
    add_std_meta_data(ea1);
    ea1.initialize();
    generate_initial_population(ea1);
    
    // run and checkpoint ea1:
    ea1.advance_epoch(10);
    std::ostringstream out;
    checkpoint_save(ea1, out);
    
    // load the saved state into ea2:
    std::istringstream in(out.str());
    checkpoint_load(ea2, in);
    
    // run each a little longer:
    ea1.advance_epoch(10);
    //	BOOST_CHECK_NE(ea1, ea2);
    ea2.advance_epoch(10);
    
    // and check them for equality:
    //	BOOST_CHECK(eq);
}
