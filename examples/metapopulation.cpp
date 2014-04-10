/* meta_population.cpp
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
#include <ea/evolutionary_algorithm.h>
#include <ea/representations/bitstring.h>
#include <ea/fitness_functions/all_ones.h>
#include <ea/cmdline_interface.h>
#include <ea/generational_models/steady_state.h>
#include <ea/datafiles/fitness.h>
#include <ea/metapopulation.h>
#include <ea/island_model.h>
using namespace ealib;

/* This example demonstrats a simple island model GA, where each individuals 
 are able to migrate among islands.  The key to understanding meta-populations 
 in EALib is that *each* island is its own, independent, instance of an
 evolutionary_algorithm.  Anything that crosses island boundaries is then 
 defined at the meta-population level.
 */

// Subpopulation definition:
typedef evolutionary_algorithm
< individual<bitstring, all_ones>
, ancestors::random_bitstring
, mutation::operators::per_site<mutation::site::bit>
, recombination::asexual
, generational_models::steady_state< >
> ea_type;

//! Metapopulation definition:
typedef metapopulation
< subpopulation<ea_type>
> mea_type;


/*! Define the EA's command-line interface.
 */
template <typename EA>
class cli : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
        add_option<REPRESENTATION_SIZE>(this);
        add_option<POPULATION_SIZE>(this);
        add_option<STEADY_STATE_LAMBDA>(this);
        add_option<MUTATION_PER_SITE_P>(this);
        add_option<TOURNAMENT_SELECTION_N>(this);
        add_option<TOURNAMENT_SELECTION_K>(this);
        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<CHECKPOINT_PREFIX>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);

        add_option<META_POPULATION_SIZE>(this);
        add_option<ISLAND_MIGRATION_PERIOD>(this);
        add_option<ISLAND_MIGRATION_RATE>(this);
    }
    
    virtual void gather_events(EA& ea) {
        add_event<island_model>(ea);
        add_event<datafiles::metapopulation_fitness_dat>(ea);
    };
};
LIBEA_CMDLINE_INSTANCE(mea_type, cli);
