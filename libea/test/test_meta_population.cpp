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

#include <ea/meta_population.h>
#include <ea/adaptive_hfc.h>
#include <ea/alps.h>

template <typename EA>
struct test_configuration : public abstract_configuration<EA> {
    void initial_population(EA& ea) {
        //        generate_ancestors(mkv_random_individual(), get<POPULATION_SIZE>(ea), ea);
    }
};

BOOST_AUTO_TEST_CASE(test_meta_population) {
    typedef meta_population<all_ones_ea, test_configuration> ea_type;    
    ea_type ea;
}


BOOST_AUTO_TEST_CASE(test_adaptive_hfc) {
    typedef meta_population<all_ones_ea, test_configuration> ea_type;
    ea_type ea;
    adaptive_hfc<ea_type> ahfc(ea);
    adaptive_hfc_datafile<ea_type> ahfcdf(ea);
}

BOOST_AUTO_TEST_CASE(test_alps) {
    typedef meta_population<all_ones_ea, test_configuration> ea_type;
    ea_type ea;
    alps<ea_type> a(ea);
}
