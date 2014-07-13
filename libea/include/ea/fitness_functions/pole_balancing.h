/* pole_balancing.h
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
#ifndef _EA_FITNESS_FUNCTIONS_POLE_BALANCING_H_
#define _EA_FITNESS_FUNCTIONS_POLE_BALANCING_H_

#include <boost/math/constants/constants.hpp>

#include <ea/fitness_function.h>
#include <ea/metadata.h>

namespace ealib {
	
    LIBEA_MD_DECL(POLE_MAXSTEPS, "ea.fitness_function.pole_balancing.max_steps", int);
    
	/*! Fitness function for 1D singple pole balancing.

     Inspired by cart_and_pole() by Richard Sutton and Charles Anderson, and
     go_cart() by Ken Stanley.   As in Stanley's version, this simulator uses 
     normalized, continous inputs instead of discretizing the input space.
     */
	struct pole_balancing : public fitness_function<unary_fitness<double> > {
        
        /*! Takes an action and the current values of the four state variables and
         updates their values by estimating the state TAU seconds later.
         
         A small change here is to treat the action as the fraction of FORCE_MAG
         that is applied, instead of always applying FORCE_MAG in some direction.
         */
        void update_cart(double action, double& x, double& x_dot, double& theta, double& theta_dot) {
            assert((action >= -1.0) && (action <= 1.0));
            const double GRAVITY=9.8;
            const double MASSCART=1.0;
            const double MASSPOLE=0.1;
            const double TOTAL_MASS=(MASSPOLE + MASSCART);
            const double LENGTH=0.5;	  /* actually half the pole's length */
            const double POLEMASS_LENGTH=(MASSPOLE * LENGTH);
            const double FORCE_MAG=10.0;
            const double TAU=0.02;	  /* seconds between state updates */
            const double FOURTHIRDS=4.0/3.0;
            
            double force = action * FORCE_MAG;
            double costheta = cos(theta);
            double sintheta = sin(theta);
            double temp = (force + POLEMASS_LENGTH * theta_dot * theta_dot * sintheta) / TOTAL_MASS;
            double thetaacc = (GRAVITY * sintheta - costheta* temp) / (LENGTH * (FOURTHIRDS - MASSPOLE * costheta * costheta / TOTAL_MASS));
            double xacc  = temp - POLEMASS_LENGTH * thetaacc * costheta / TOTAL_MASS;
            
            // Update the four state variables using Euler's method:
            x  += TAU * x_dot;
            x_dot += TAU * xacc;
            theta += TAU * theta_dot;
            theta_dot += TAU * thetaacc;
        }
        
        /*! Evaluate the fitness of a single individual.
         */
        template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            const double twelve_degrees=12.0 * boost::math::constants::pi<double>() / 180.0;
            double x=0.0; // cart position, meters
            double x_dot=0.0; // cart velocity
            double theta=0.0; // pole angle, radians
            double theta_dot=0.0; // pole angular velocity
            double input[5]; // inputs to the phenotype
            typename EA::phenotype_type &P = ealib::phenotype(ind, ea); // phenotype; ANN, MKV, etc.
            int maxsteps = get<POLE_MAXSTEPS>(ea);
            
            // update the phenotype and cart:
            for(int i=0; i<maxsteps; ++i) {
                input[0] = 1.0;
                input[1] = (x + 2.4) / 4.8;
                input[2] = (x_dot + 0.75) / 1.5;
                input[3] = (theta + twelve_degrees) / 0.41;
                input[4] = (theta_dot + 1.0) / 2.0;

                // code to update the inputs and get outputs from the phenotype goes here, e.g.:
                P.update(input, input+5);
                update_cart(*P.begin_output(), x, x_dot, theta, theta_dot);
                
                if((x < -2.4) || (x > 2.4) // out of bounds
                   || (theta < -twelve_degrees) || (theta > twelve_degrees)) { // dropped the pole
                    return static_cast<double>(i);
                }
            }
            
            return static_cast<double>(maxsteps);
        }
	};
    
} // ealib

#endif
