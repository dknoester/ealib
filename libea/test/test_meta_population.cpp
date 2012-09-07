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

template <typename EA>
struct test_configuration : public abstract_configuration<EA> {
    void initial_population(EA& ea) {
        //        generate_ancestors(mkv_random_individual(), get<POPULATION_SIZE>(ea), ea);
    }
};

//typename EA,
//template <typename> class ConfigurationStrategy,
//template <typename> class EventHandler=event_handler,
//typename MetaData=meta_data,
//typename RandomNumberGenerator=ea::default_rng_type>
//class meta_population {

/*! 
 */
BOOST_AUTO_TEST_CASE(test_meta_population) {
    typedef meta_population<all_ones_ea, test_configuration> ea_type;
    
    ea_type ea;
}


BOOST_AUTO_TEST_CASE(test_adaptive_hfc) {
    
    
}
