/* test.h
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
#ifndef _TEST_H_
#define _TEST_H_

#include <boost/test/unit_test.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <sstream>

#include <ea/evolutionary_algorithm.h>
#include <ea/genome_types/bitstring.h>
#include <ea/fitness_functions/all_ones.h>
#include <ea/generational_models/steady_state.h>
#include <ea/line_of_descent.h>
using namespace ealib;

typedef evolutionary_algorithm
< direct<bitstring>
, all_ones
, mutation::operators::per_site<mutation::site::bitflip>
, recombination::two_point_crossover
, generational_models::steady_state< >
, ancestors::random_bitstring
> all_ones_ea;

typedef evolutionary_algorithm
< direct<bitstring>
, all_ones
, mutation::operators::per_site<mutation::site::bitflip>
, recombination::two_point_crossover
, generational_models::steady_state< >
, ancestors::random_bitstring
, dont_stop
, fill_population
, default_lifecycle
, lod_trait
> all_ones_lod_ea;

metadata build_ea_md();

#endif
