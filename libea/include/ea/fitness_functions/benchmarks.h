/* benchmarks.h
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
#ifndef _EA_FITNESS_FUNCTIONS_BENCHMARKS_H_
#define _EA_FITNESS_FUNCTIONS_BENCHMARKS_H_

#include <algorithm>
#include <ea/fitness_function.h>

namespace ealib {
    
    LIBEA_MD_DECL(BENCHMARKS_FUNCTION, "ea.fitness_function.benchmarks_function", int);
    

    /* All of the benchmark fitness functions contained in this file are courtesy
     of GENITOR: http://www.cs.colostate.edu/~genitor/functions.html
     */

    /*! RANA
     */
	struct rana : public fitness_function<unary_fitness<double,minimizeS>, constantS, deterministicS> {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            typename EA::genome_type& params = ind.genome();
            
            double p1=params[0];
            double p2=params[1];
            double sum=0.0;
            int dim = params.size();
            for(int i=0; i<dim-1; i++){
                p1=params[i];
                p2=params[i+1];
                
                sum += (p1* sin(sqrt(fabs(p2+1.0-p1)))* cos(sqrt(fabs(p1+p2+1.0))) +
                        (p2+1.0)* cos(sqrt(fabs(p2+1.0-p1)))* sin(sqrt(fabs(p1+p2+1.0))));
            }
            
            return sum;
        }
    };
	
    /*! GRIEWANGK
     */
	struct griewangk : public fitness_function<unary_fitness<double,minimizeS>, constantS, deterministicS> {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            typename EA::genome_type& params = ind.genome();
            
            assert(params.size()>0);
            double sum=0.0;
            double prod=1.0;
            
            for(int i=0;i<int(params.size());i++) {
                double xi=params[i];
                sum+=xi*xi/4000.0;
                prod*=cos(xi/sqrt(double(i+1)));
            }
            return 1.0+sum-prod;
        }
    };
    
    /*! ROSENBROCK
     */
	struct rosenbrock : public fitness_function<unary_fitness<double,minimizeS>, constantS, deterministicS> {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            typename EA::genome_type& params = ind.genome();
            
            double x1=params[0];
            double x2=params[1];
            double sum=0.0;
            int dim = params.size();
            for(int i=0; i<dim-1; i++){
                x1=params[i];
                x2=params[i+1];
                double sq_x1=x1*x1;
                double diff_x1=1.0-x1;
                sum=sum+(100.0*((sq_x1-x2)*(sq_x1-x2)))+(diff_x1*diff_x1);
            }
            return sum;
        }
    };

    /*! SCHWEFEL
     */
	struct schwefel : public fitness_function<unary_fitness<double,minimizeS>, constantS, deterministicS> {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            typename EA::genome_type& params = ind.genome();
            
            assert(params.size()>0);
            double result=0.0;
            
            for(int i=0;i<int(params.size());i++) {
                double xi=params[i];
                result+=(-xi)*sin(sqrt(fabs(xi)));
            }
            return result;
        }
    };

    /*! F101
     */
	struct f101 : public fitness_function<unary_fitness<double,minimizeS>, constantS, deterministicS> {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            typename EA::genome_type& params = ind.genome();
            
            double x=params[0];
            double y=params[1];
            double sum=0.0;
            int dim = params.size();
            for(int i=0; i<dim-1; i++){
                x=params[i];
                y=params[i+1];
                sum+= (-x*sin(sqrt(fabs(x-(y+47))))-(y+47)*sin(sqrt(fabs(y+47+(x/2)))));
            }
            return sum;         }
    };

    /*! F8F2 - Fix!
     */
	struct f8f2 : public fitness_function<unary_fitness<double,minimizeS>, constantS, deterministicS> {
		template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            typename EA::genome_type& params = ind.genome();
            
            double p1=params[0];
            double p2=params[1];
            double sum = 0.0;
            
            int dim = params.size();
            for( int i=0; i < dim-1; i++ ){
                p1 = params[i];
                p2 = params[i+1];
                
                std::vector<double> f2_args;
                f2_args.push_back( p1 );
                f2_args.push_back( p2 );
                
                double f2_res = 0; // rosenbrock_eval( f2_args );
                
                // Shift/Scale range of F2 to be the domain of F8
                // Range of F2 ~ [0, 3900]
                f2_res /= 3900;
                f2_res *= 1024;
                f2_res -= 512;
                
                std::vector<double> f8_args;
                f8_args.push_back( f2_res );
                
                //sum += griewangk_eval( f8_args );
            }
            return sum;
        }
    };
    
    
    /*! benchmarks - this fitness function allows you to select which fitness function to use - BENCHMARKS_FUNCTION
     */
    struct benchmarks : public fitness_function<unary_fitness<double,minimizeS>, constantS, deterministicS> {
        
        template <typename Individual, typename EA>
		double operator()(Individual& ind, EA& ea) {
            switch(get<BENCHMARKS_FUNCTION>(ea,-1)) {
                case 0: {
                    return _ra(ind, ea);
                }
                case 1: {
                    return _gr(ind, ea);
                }
                case 2: {
                    return _ro(ind, ea);
                }
                case 3: {
                    return _sc(ind, ea);
                }
                case 4: {
                    return _f1(ind, ea);
                }
                default: {
                    throw bad_argument_exception("benchmarks.h: unknown benchmark function.");
                }
            }
        }
        
        // benchmarking functions:
        rana _ra;
        griewangk _gr;
        rosenbrock _ro;
        schwefel _sc;
        f101 _f1;
    };

} // ealib

#endif
