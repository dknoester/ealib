/* test_metadata.cpp
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

BOOST_AUTO_TEST_CASE(test_md) {
    all_ones_ea ea1(build_ea_md()), ea2;
	put<POPULATION_SIZE>(10,ea1);
	put<MUTATION_PER_SITE_P>(1.0,ea1);
	
	std::ostringstream out;
	boost::archive::xml_oarchive oa(out);
	oa << BOOST_SERIALIZATION_NVP(ea1);
	
	std::istringstream in(out.str());
	boost::archive::xml_iarchive ia(in);
	ia >> BOOST_SERIALIZATION_NVP(ea2);
	
	BOOST_CHECK_EQUAL(get<POPULATION_SIZE>(ea1), 10u);
	BOOST_CHECK_EQUAL(get<POPULATION_SIZE>(ea1), get<POPULATION_SIZE>(ea2));
	BOOST_CHECK_EQUAL(get<MUTATION_PER_SITE_P>(ea1), 1.0);
	BOOST_CHECK_EQUAL(get<MUTATION_PER_SITE_P>(ea1), get<MUTATION_PER_SITE_P>(ea2));
}
