/* test_qhfc.cpp
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
#include <ea/fitness_functions/all_ones.h>
#include <ea/qhfc.h>


BOOST_AUTO_TEST_CASE(test_qhfc) {
    typedef qhfc
    < direct<bitstring>
    , all_ones
    , mutation::operators::per_site<mutation::site::bit>
    , recombination::two_point_crossover
    , ancestors::random_bitstring
    > ea_type;

    metadata M;
    put<POPULATION_SIZE>(50,M);
    put<METAPOPULATION_SIZE>(5,M);
	put<REPRESENTATION_SIZE>(100,M);
	put<MUTATION_PER_SITE_P>(0.0005,M);
    put<QHFC_POP_SCALE>(0.8,M);
    put<QHFC_BREED_TOP_FREQ>(2,M);
    put<QHFC_DETECT_EXPORT_NUM>(2,M);
    put<QHFC_PERCENT_REFILL>(0.25,M);
    put<QHFC_CATCHUP_GEN>(20,M);
    put<QHFC_NO_PROGRESS_GEN>(2,M);
    put<CHECKPOINT_OFF>(0,M);
    
    ea_type ea(M);
    generate_initial_population(ea);
    ea.lifecycle().advance_epoch(10,ea);
}
