/* universal_sampling.h
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
#ifndef _EA_SELECTION_UNIVERSAL_SAMPLING_H_
#define _EA_SELECTION_UNIVERSAL_SAMPLING_H_

#include <ea/attributes.h>
#include <ea/selection.h>

namespace ealib {
	namespace selection {
        
		/*! Stochastic universal sampling selection.
		 
         This strategy selects individuals based on some attribute, but does so
         by selects based on evenly-spaced intervals.  This prevents a single
         individual from dominating selection.
         
         (Think "comb" over a number line.)
		 */
        template <typename AttributeAccessor=access::fitness_accessor>
		struct universal_sampling {
            typedef AttributeAccessor acc_type; //!< Accessor for proportionate selection.
            
			//! Initializing constructor.
			template <typename Population, typename EA>
			universal_sampling(std::size_t n, Population& src, EA& ea) : _sum(0.0), _offset(0.0) {
                for(typename Population::iterator i=src.begin(); i!=src.end(); ++i) {
                    _sum += static_cast<double>(_acc(**i,ea));
                }
                if(_sum == 0.0) {
                    _sum = src.size();
                    _offset = 1.0;
                }
			}
            
			/*! Select n individuals via universal sampling.
             */
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                // fixed distance (in attribute-space) between individuals (the comb):
                double p = _sum/static_cast<double>(n);
                
                // where do we start?
                double r = ea.rng().uniform_real(0.0, p);
                double running = 0.0;

                for( ; n>0; --n) {
                    
                }
                
                
                
                SUS(Population, N)
                F := total fitness of population
                N := number of offspring to keep
                P := distance between the pointers (F/N)
                Start := random number between 0 and P
                Pointers := [Start + i*P | i in [0..N-1]]
                return RWS(Population,Pointers)
                
                RWS(Population, Points)
                Keep = []
                i := 0
                for P in Points
                while fitness of Population[i] < P
                i++
                add Population[i] to Keep
                return Keep
                
                
                // build a whole bunch of random numbers, and sort them:
                std::vector<double> rnums(n);
                std::generate(rnums.begin(), rnums.end(), ea.rng().uniform_real_rng(0,1.0));
                std::sort(rnums.begin(), rnums.end()); // ascending
                
                // since the rnums are sorted, we can scan through linearly.
                // since the fsum is monotonically increasing, all we need to do is scale
                // the current rnum by the fraction of fitness we're currently looking at:
                typename Population::iterator p=src.begin();
                double running=static_cast<double>(_acc(**p,ea) + _offset);
                for(std::vector<double>::iterator i=rnums.begin(); i!=rnums.end(); ++i) {
                    // while our (the fraction of) running fitness is strictly less than the current
                    // random number, go to the next individual in the population:
                    while((running/_sum) < (*i)) {
                        ++p;
                        assert(p!=src.end());
                        running += static_cast<double>(_acc(**p,ea) + _offset);
                    }
                    
                    // ok, running fitness is >= random number; select the p'th
                    // individual for replication:
                    dst.push_back(*p);
                }
            }
            
            acc_type _acc; //!< Accessor for value used for proportionate selection.
			double _sum; //!< Sum of fitnesses in the population being selected from.
            double _offset; //!< Amount by which to offset the attribute being selected over.
		};
        
	} // selection
} // ealib

#endif
