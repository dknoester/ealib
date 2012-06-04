#ifndef _EA_SELECTION_FITNESS_PROPORTIONAL_H_
#define _EA_SELECTION_FITNESS_PROPORTIONAL_H_

#include <ea/algorithm.h>
#include <ea/interface.h>


namespace ea {
	namespace selection {
		
		/*! Fitness proportional selection.
		 
		 This selection strategy organizes the individuals by fitness. For each member of the
		 next generation, a random number (between 0 and the sum of the individuals' fitnesses 
		 is selected) is generated. This number indexes an individual, which is added to the 
		 next generation. 
		 */
		struct fitness_proportional {
			//! Initializing constructor.
			template <typename Population, typename EA>
			fitness_proportional(std::size_t n, Population& src, EA& ea) : _sum(0.0) {
				_sum=accumulate_fitness(src.begin(), src.end(), ea);
                assert(_sum > 0.0);
			}
            
            //! Adjust the sum of fitnesses by val.
            void adjust(double val) {
                _sum += val;
            }
			
			//! Select n individuals via fitness-proportional selection.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                assert(_sum > 0.0);
				typename EA::rng_type::real_rng_type my_rng = ea.rng().uniform_real_rng(0.0, _sum);
				
				for( ; n>0; n--) {
					dst.append(*algorithm::roulette_wheel(my_rng(), src.begin(), src.end(), ea));
				}
			}

			double _sum; //!< Sum of fitnesses in the population being selected from.
		};
		
	} // selection
} // ealib

#endif
