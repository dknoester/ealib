#ifndef _EA_SELECTION_ELITISM_H_
#define _EA_SELECTION_ELITISM_H_

#include <algorithm>
#include <iterator>
#include <ea/algorithm.h>
#include <ea/interface.h>

namespace ea {
	namespace selection {
		
		/*! Stacks elitism on top of another selection strategy.
		 
		 This selection strategy "stacks" with others; that is, it must be used in
		 conjunction with another selection strategy, such as tournament_selection.
		 Elitism augments that selection strategy by explicitly preserving N "elite"
		 (high-fitness) individuals.  Those selected are *still* maintained as part
		 of the source population from which the embedded selection strategy draws its
		 own selected individuals.
		 */
 		template <typename SelectionStrategy>
		struct elitism {
			typedef SelectionStrategy embedded_selection_type;

			//! Initializing constructor.
			template <typename Population, typename EA>
			elitism(std::size_t n, Population& src, EA& ea) : _embedded(src,ea) {
			}
            
			/*! Preserve the elite individuals from the src population.
             */
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
				std::size_t e = get<ELITISM_N>(ea);
                assert(n > e);
                _embedded(src, dst, n-e, ea);
                
                // now, append the e most-fit individuals:
                std::sort(src.begin(), src.end(), algorithm::fitness_comp<EA>(ea));
                typename Population::reverse_iterator rl=src.rbegin();
                std::advance(rl, e);
                dst.append(src.rbegin(), rl);
			};

			embedded_selection_type _embedded; //!< Underlying selection strategy.
		};

	} // selection
} // ealib

#endif
