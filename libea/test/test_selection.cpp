/* test_selection.cpp
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

#include <ea/interface.h>
#include <ea/selection/truncation.h>
#include <ea/selection/elitism.h>
#include <ea/selection/proportionate.h>
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
	test_selection_strategy<ea::selection::proportionate< > >();
	test_selection_strategy<ea::selection::random>();
	test_selection_strategy<ea::selection::tournament< > >();
	test_selection_strategy<ea::selection::elitism<ea::selection::tournament< > > >();
}
