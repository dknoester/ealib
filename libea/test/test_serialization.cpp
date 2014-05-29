/* test_checkpoint.cpp
 * 
 * This file is part of EALib.
 * 
 * Copyright 2014 David B. Knoester.
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
#include <ea/archive.h>


/*! Test of EA checkpointing.
 */
BOOST_AUTO_TEST_CASE(test_checkpoint) {
    all_ones_ea ea1, ea2;
    add_std_meta_data(ea1);
    ea1.lifecycle().prepare_new(ea1);
    
    // run and checkpoint ea1:
    ea1.lifecycle().advance_epoch(10,ea1);
    std::ostringstream out;
    ea1.lifecycle().save_checkpoint(out, ea1);
    
    // load the saved state into ea2:
    std::istringstream in(out.str());
    ea2.lifecycle().load_checkpoint(in, ea2);
    
    // run each a little longer:
    ea1.lifecycle().advance_epoch(10,ea1);
    //	BOOST_CHECK_NE(ea1, ea2);
    ea2.lifecycle().advance_epoch(10,ea2);
    
    // check that the individuals in ea1 are pretty much the same as the individuals in ea2:
    for(all_ones_ea::iterator i=ea1.begin(), j=ea2.begin(); i!=ea1.end(); ++i, ++j) {
        BOOST_CHECK(ealib::fitness(*i,ea1) == ealib::fitness(*j,ea2));
        BOOST_CHECK(get<IND_NAME>(*i) == get<IND_NAME>(*j));
    }
}

BOOST_AUTO_TEST_CASE(test_replicability) {
    all_ones_ea ea1, ea2;
    add_std_meta_data(ea1);
    add_std_meta_data(ea2);
    ea1.rng().reset(42);
    ea2.rng().reset(42);
    
    ea1.lifecycle().prepare_new(ea1);
    ea2.lifecycle().prepare_new(ea2);
    
    ea1.lifecycle().advance_epoch(100,ea1);
    ea2.lifecycle().advance_epoch(100,ea2);
    
    // check that the individuals in ea1 are pretty much the same as the individuals in ea2:
    for(all_ones_ea::iterator i=ea1.begin(), j=ea2.begin(); i!=ea1.end(); ++i, ++j) {
        BOOST_CHECK(ealib::fitness(*i,ea1) == ealib::fitness(*j,ea2));
        BOOST_CHECK(get<IND_NAME>(*i) == get<IND_NAME>(*j));
        BOOST_CHECK(ea1.rng() == ea2.rng());
    }
}

/*! Test of EA archiving.
 */
BOOST_AUTO_TEST_CASE(test_archive) {
    all_ones_ea ea1, ea2;
    add_std_meta_data(ea1);
    ea1.lifecycle().prepare_new(ea1);
    
    // run and archive ea1:
    ea1.lifecycle().advance_epoch(10,ea1);
    std::ostringstream out;
    save_archive(out, ea1);
    
    // load the archive into ea2:
    std::istringstream in(out.str());
    load_archive(in, ea2);
    
    // check that the individuals in ea1 are pretty much the same as the individuals in ea2:
    for(all_ones_ea::iterator i=ea1.begin(), j=ea2.begin(); i!=ea1.end(); ++i, ++j) {
        BOOST_CHECK(ealib::fitness(*i,ea1) == ealib::fitness(*j,ea2));
        BOOST_CHECK(get<IND_NAME>(*i) == get<IND_NAME>(*j));
    }
}
