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
#include <ea/cmdline_interface.h>
#include <ea/datafiles/fitness.h>
#include <ea/genome_types/realstring.h>
#include <ea/algorithm.h>
#include <ea/analysis/dominant.h>
using namespace ealib;

struct pid_control : public fitness_function<unary_fitness<double,minimizeS> > {
    template <typename Individual, typename EA>
    double operator()(Individual& ind, EA& ea, datafile* df=0, datafile* df2=0) {
        typename EA::genome_type& genome = ind.genome();
        double kp = genome[0]; // proporational gain
        double ki = genome[1]; // integral gain
        double kd = genome[2]; // derivative gain
        
        if(df2) {
            df2->write(ea.current_update()).write("kp").write(kp).endl();
            df2->write(ea.current_update()).write("ki").write(ki).endl();
            df2->write(ea.current_update()).write("kd").write(kd).endl();
        }
        
        double int_e=0.0; // integrated error
        double et=0.0; // error at time t
        double etm1=0.0; // error at time t-1
        double sse=0.0; // sum-squared-error (for fitness calculation)
        
        // The reference signal for this simulation:
        //
        // 1.0  |
        // 0.75 |----
        // 0.5  |    |     -----
        // 0.25 |    L_____|
        // 0.0  |
        // ---------------------
        //    t  0   100   200
        double reference = 0.75; // reference signal
        double output=0.0; // controller output at time t
        double outputm1=0.0; // controller output at time t-1
        double measured=0.0; // sensor input
        
        // simulate for 250 time steps (ms):
        for(int t=0; t<250; ++t) {
            // reference signal:
            // step from 0.75->0.25 @ t==100
            if(t == 50) { reference = 0.25; }
            // step from 0.25->0.5 @ t==200
            if(t == 100) { reference = 0.5; }
            // step from 0.5->1.0 @ t==200
            if(t == 150) { reference = 1.0; }
            // step from 1.0->0.25 @ t==200
            if(t == 200) { reference = 0.25; }
            
            et = reference - measured; // instantaneous error
            int_e += et; // integrated error
            sse += et*et; // sum-squared error
            
            double p = kp * et; // proportional term
            double i = ki * int_e; // integral term
            double d = kd * (et - etm1); // derivative term
            output = p + i + d; // controller output
            output = std::max(output, 0.0);
            
            if(df) {
                df->write(t).write("reference").write(reference).endl();
                df->write(t).write("measured").write(measured).endl();
                df->write(t).write("output").write(output).endl();
            }
            
            // ---- end of controller ----
            
            // update plant:
            measured *= 0.5; // steady-state 10% reduction in sensor value (e.g., fuel consumption)
            measured += output; // change due to active control (e.g., fuel pump)
            measured = algorithm::clip(measured, 0.0, 1.0);

            // end of loop; rotate error and output @ time t -> t-1
            std::swap(et, etm1);
            std::swap(output, outputm1);
        }
        
        double rmse = sqrt(sse / 250.0);
        if(std::isnan(rmse)) {
            return std::numeric_limits<double>::infinity();
        }
        return rmse;
    }
};


//! Save the fitness values of all individuals in the population.
LIBEA_ANALYSIS_TOOL(pid_detail) {
    datafile df("pid_detail.dat");
    df.add_field("time").add_field("var").add_field("value");
    
    typename EA::iterator i=analysis::dominant(ea);
    ea.fitness_function()(*i, ea, &df);
}


template <typename EA>
struct pid_movie_dat : record_statistics_event<EA> {
    pid_movie_dat(EA& ea) : record_statistics_event<EA>(ea) {
    }
    
    virtual ~pid_movie_dat() {
    }
    
    virtual void operator()(EA& ea) {
        datafile df("pid_movie_" + boost::lexical_cast<std::string>(ea.current_update()) + ".dat");
        df.add_field("time").add_field("var").add_field("value");
        
        typename EA::iterator i=analysis::dominant(ea);
        ea.fitness_function()(*i, ea, &df);
    }
};

template <typename EA>
struct pid_params_dat : record_statistics_event<EA> {
    pid_params_dat(EA& ea) : record_statistics_event<EA>(ea), _df("pid_params.dat") {
        _df.add_field("update").add_field("var").add_field("value");
    }
    
    virtual ~pid_params_dat() {
    }
    
    virtual void operator()(EA& ea) {
        typename EA::iterator i=analysis::dominant(ea);
        ea.fitness_function()(*i, ea, 0, &_df);
    }
    
    datafile _df;
};


typedef evolutionary_algorithm
< direct<realstring>
, pid_control
, mutation::operators::per_site<mutation::site::mutation_pair<mutation::site::uniform_real, mutation::site::relative_normal_real> >
, recombination::asexual
, generational_models::steady_state<selection::proportionate< >, selection::rank< > >
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
        add_option<MUTATION_PER_SITE_P>(this);
        add_option<MUTATION_NORMAL_REAL_VAR>(this);
        add_option<MUTATION_UNIFORM_REAL_MIN>(this);
        add_option<MUTATION_UNIFORM_REAL_MAX>(this);
        add_option<STEADY_STATE_LAMBDA>(this);
        add_option<RUN_UPDATES>(this);
        add_option<RUN_EPOCHS>(this);
        add_option<CHECKPOINT_PREFIX>(this);
        add_option<RNG_SEED>(this);
        add_option<RECORDING_PERIOD>(this);
    }
    
    virtual void gather_tools() {
        add_tool<pid_detail>(this);
    }
    
    virtual void gather_events(EA& ea) {
        add_event<datafiles::fitness_dat>(ea);
//        add_event<pid_movie_dat>(ea);
//        add_event<pid_params_dat>(ea);
    };
};
LIBEA_CMDLINE_INSTANCE(ea_type, cli);
