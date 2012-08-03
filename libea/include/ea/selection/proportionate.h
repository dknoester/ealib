/* proportionate.h
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
#ifndef _EA_SELECTION_PROPORTIONATE_H_
#define _EA_SELECTION_PROPORTIONATE_H_

#include <ea/attributes.h>
#include <ea/interface.h>
#include <ea/selection.h>

namespace ea {
	namespace selection {
        
		/*! {roportionate selection.
		 
         This strategy selects individuals proportionately by some attribute,
         usually fitness.
         
         Note: There is a member variable here called "offset," whose purpose is
         to offset the selection attribute when the sum of that attribute is 0.0.
         For example, if all fitnesses in the population are 0.0, then FPS normally
         breaks (divide by zero).  The offset prevents users of proportionate selection
         from having to deal with this.
		 */
        template <typename AttributeAccessor=attributes::fitness>
		struct proportionate {
            typedef AttributeAccessor acc_type; //!< Accessor for proportionate selection.
            
			//! Initializing constructor.
			template <typename Population, typename EA>
			proportionate(std::size_t n, Population& src, EA& ea) : _sum(0.0), _offset(0.0) {
                for(typename Population::iterator i=src.begin(); i!=src.end(); ++i) {
                    _sum += static_cast<double>(_acc(**i));
                }
                if(_sum == 0.0) {
                    _sum = src.size();
                    _offset = 1.0;
                }
			}

			/*! Select n individuals via fitness-proportionate selection.
             
             We go through a few hoops here in order to avoid n linear time lookups...
             */
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                // build a whole bunch of random numbers, and sort them:
                std::vector<double> rnums(n);
                std::generate(rnums.begin(), rnums.end(), ea.rng().uniform_real_rng(0,1.0));
                std::sort(rnums.begin(), rnums.end()); // ascending
                
                // since the rnums are sorted, we can scan through linearly.
                // since the fsum is monotonically increasing, all we need to do is scale
                // the current rnum by the fraction of fitness we're currently looking at:
                typename Population::iterator p=src.begin();
                double running=static_cast<double>(_acc(**p) + _offset);
                for(std::vector<double>::iterator i=rnums.begin(); i!=rnums.end(); ++i) {
                    // while our (the fraction of) running fitness is strictly less than the current
                    // random number, go to the next individual in the population:
                    while((running/_sum) < (*i)) {
                        ++p;
                        assert(p!=src.end());
                        running += static_cast<double>(_acc(**p) + _offset);
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
		
        
        
        /*! Roulette wheel selection.
		 
		 Returns an iterator selected from the range [f,l), based on the sum of values
		 of the items in the sequence.
		 
		 If a value could not be found, return l.
		 */
		template <typename T, typename InputIterator, typename EA>
		InputIterator roulette_wheel(const T& target, InputIterator f, InputIterator l, EA& ea) {
			T running=0.0;
			for( ; f!=l; ++f) {
				running += static_cast<T>(ind(f,ea).fitness());
				if(running >= target) {
					return f;
				}
			}			
			return l;
		}
        
        
        
	} // selection
} // ealib

#endif
