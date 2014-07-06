/* pole_balancing.cpp
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
#include <ea/evolutionary_algorithm.h>
#include <ea/generational_models/moran_process.h>
#include <ea/genome_types/realstring.h>
#include <ea/fitness_functions/pole_balancing.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
#include <ea/translation.h>
#include <ann/feed_forward.h>
#include <ann/neuroevolution.h>
using namespace ealib;

typedef evolutionary_algorithm
< indirect<realstring, ann::feed_forward< >, ann::neural_network_reconstruction>
, pole_balancing
, mutation::operators::per_site<mutation::site::relative_normal_real>
, recombination::asexual
, generational_models::moran_process< >
, ancestors::uniform_real
> ea_type;

/*! Define the EA's command-line interface.
 */
template <typename EA>
class cli : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
        add_option<REPRESENTATION_SIZE>(this);
        add_option<POPULATION_SIZE>(this);
        add_option<MORAN_REPLACEMENT_RATE_P>(this);
        add_option<MUTATION_PER_SITE_P>(this);
        add_option<MUTATION_NORMAL_REAL_VAR>(this);
        add_option<MUTATION_UNIFORM_REAL_MIN>(this);
        add_option<MUTATION_UNIFORM_REAL_MAX>(this);
        
        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<CHECKPOINT_PREFIX>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);
        add_option<POLE_MAXSTEPS>(this);

        add_option<ann::ANN_INPUT_N>(this);
        add_option<ann::ANN_OUTPUT_N>(this);
        add_option<ann::ANN_HIDDEN_N>(this);
    }
    
    
    virtual void gather_tools() {
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
    };
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
