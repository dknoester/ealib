/* test_checkpoint.cpp
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

/*! Test of EA checkpointing.
 */
BOOST_AUTO_TEST_CASE(ealib_checkpoint) {
    all_ones_ea ea1, ea2;
    add_std_meta_data(ea1);
    ea1.initialize();
    ea1.generate_initial_population();
    
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
