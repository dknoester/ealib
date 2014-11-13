/* test.cpp
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
#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MAIN

#include "test.h"

metadata build_ea_md() {
	metadata md;
	put<POPULATION_SIZE>(1024,md);
	put<STEADY_STATE_LAMBDA>(2,md);
	put<REPRESENTATION_SIZE>(10,md);
	put<MUTATION_PER_SITE_P>(0.1,md);
	put<TOURNAMENT_SELECTION_N>(2,md);
	put<TOURNAMENT_SELECTION_K>(1,md);
    put<CHECKPOINT_OFF>(0,md);
    put<METAPOPULATION_SIZE>(5,md);
    put<RUN_UPDATES>(10,md);
    return md;
}
