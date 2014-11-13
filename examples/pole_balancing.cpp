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
#include <ea/generational_models/steady_state.h>
#include <ea/selection/rank.h>
#include <ea/fitness_functions/pole_balancing.h>
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
#include <ann/basic_neural_network.h>
#include <ea/ann/neuroevolution.h>
#include <ea/analysis/dominant.h>
using namespace ealib;


template <typename EA>
struct pole_movie_dat : record_statistics_event<EA> {
    pole_movie_dat(EA& ea) : record_statistics_event<EA>(ea) {
    }
    
    virtual ~pole_movie_dat() {
    }
    
    virtual void operator()(EA& ea) {
        datafile df("pole_movie_" + boost::lexical_cast<std::string>(ea.current_update()) + ".dat");
        df.add_field("time").add_field("x").add_field("theta");
        
        typename EA::iterator i=analysis::dominant(ea);
        typename EA::rng_type rng(get<FF_RNG_SEED>(*i));
        ea.fitness_function()(*i, rng, ea, &df);
    }
};


typedef evolutionary_algorithm
< direct<ann::basic_neural_network< > >
, pole_balancing
, mutation::operators::weight_matrix
, recombination::asexual
, generational_models::steady_state<selection::proportionate< >, selection::rank< > >
, ancestors::random_weight_neural_network
> ea_type;


/*! Define the EA's command-line interface.
 */
template <typename EA>
class cli : public cmdline_interface<EA> {
public:
    virtual void gather_options() {
        add_option<REPRESENTATION_SIZE>(this);
        add_option<POPULATION_SIZE>(this);
        add_option<MUTATION_PER_SITE_P>(this);
        add_option<MUTATION_NORMAL_REAL_VAR>(this);
        add_option<STEADY_STATE_LAMBDA>(this);

        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);
        add_option<POLE_MAXSTEPS>(this);

        add_option<ANN_INPUT_N>(this);
        add_option<ANN_OUTPUT_N>(this);
        add_option<ANN_HIDDEN_N>(this);
    }
    
    virtual void gather_tools() {
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
        //        add_event<pole_movie_dat>(ea);
    };
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
