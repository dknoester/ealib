/* mp_all_ones.cpp
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
#include <ea/meta_population.h>
#include <ea/evolutionary_algorithm.h>
#include <ea/fitness_functions/all_ones.h>
#include <ea/representations/numeric_vector.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/generation_fitness.h>
#include <ea/adaptive_hfc.h>
#include <ea/alps.h>
using namespace ea;

template <typename EA>
struct configuration : public abstract_configuration<EA> {
    //! Called to generate the initial EA population.
    void initial_population(EA& ea) {
        generate_ancestors(ancestors::random_bitstring(), get<POPULATION_SIZE>(ea) - ea.population().size(), ea);
    }
};


//! Evolutionary algorithm definition.
typedef evolutionary_algorithm<
bitstring,
mutation::per_site<mutation::bitflip>,
all_ones,
configuration,
recombination::asexual,
generational_models::steady_state<selection::proportionate< >, selection::tournament< > >
> ea_type;


template <typename EA>
struct mp_configuration : public abstract_configuration<EA> {
};


typedef meta_population<ea_type, mp_configuration> mea_type;

/*! Define the EA's command-line interface.
 */
template <typename EA>
class ones : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
        // ea options
        add_option<META_POPULATION_SIZE>(this);
        add_option<REPRESENTATION_SIZE>(this);
        add_option<POPULATION_SIZE>(this);
        add_option<REPLACEMENT_RATE_P>(this);
        add_option<MUTATION_PER_SITE_P>(this);
        add_option<TOURNAMENT_SELECTION_N>(this);
        add_option<TOURNAMENT_SELECTION_K>(this);
        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<CHECKPOINT_ON>(this);
        add_option<CHECKPOINT_PREFIX>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);
        
        add_option<INITIALIZATION_PERIOD>(this);
        add_option<EXCHANGE_INDIVIDUALS_PERIOD>(this);
        add_option<ADMISSION_UPDATE_PERIOD>(this);
        add_option<MIN_REMAIN>(this);
    }
    
    virtual void gather_tools() {
    }
    
    virtual void gather_events(EA& ea) {
        add_event<alps>(this, ea);
        add_event<alps_datafile>(this, ea);
    };
};
LIBEA_CMDLINE_INSTANCE(mea_type, ones);
