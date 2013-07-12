/* test_meta_population.cpp
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
#include <ea/generational_models/crowding.h>
#include <ea/generational_models/qhfc.h>
#include <ea/meta_population.h>

template <typename EA>
struct mp_configuration : public abstract_configuration<EA> {
};

BOOST_AUTO_TEST_CASE(test_meta_population) {
    typedef meta_population<all_ones_ea, mp_configuration> mea_type;
    mea_type M;
}


BOOST_AUTO_TEST_CASE(test_qhfc) {
    typedef evolutionary_algorithm<
    bitstring,
    mutation::operators::per_site<mutation::site::bit>,
    all_ones,
    configuration,
    recombination::two_point_crossover,
    generational_models::deterministic_crowding< > > ea_type;
    
    typedef meta_population<ea_type, mp_configuration, generational_models::qhfc> mea_type;
    
    mea_type M;
    M.configure();
    put<POPULATION_SIZE>(50,M);
    put<META_POPULATION_SIZE>(5,M);
	put<REPRESENTATION_SIZE>(100,M);
	put<MUTATION_PER_SITE_P>(0.0005,M);
	put<ELITISM_N>(1,M);
    put<QHFC_BREED_TOP_FREQ>(2,M);
    put<QHFC_DETECT_EXPORT_NUM>(2,M);
    put<QHFC_PERCENT_REFILL>(0.25,M);
    put<QHFC_CATCHUP_GEN>(20,M);
    put<QHFC_NO_PROGRESS_GEN>(2,M);

    M.initialize();
    M.initial_population();
    lifecycle::advance_epoch(10,M);    
}
