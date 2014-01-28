/* test_genetic_algorithm.cpp
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
#include <ea/genetic_algorithm.h>

BOOST_AUTO_TEST_CASE(test_genetic_algorithm) {    
    typedef genetic_algorithm
    < bitstring
    , all_ones
    , ancestors::random_bitstring
    , mutation::operators::per_site<mutation::site::bitflip>
    , recombination::two_point_crossover
    , generational_models::steady_state< >
    > ea_type;
    
    ea_type ea;
    add_std_meta_data(ea);
    
    lifecycle::prepare_new(ea);
    lifecycle::advance_epoch(10,ea);
}
