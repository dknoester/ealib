#ifndef _EA_SELECTION_TRUNCATION_H_
#define _EA_SELECTION_TRUNCATION_H_

#include <algorithm>
#include <iterator>
#include <ea/algorithm.h>

namespace ea {
	namespace selection {
		
		/*! Truncation selection.
		 
		 This selection method truncates a population by removing low-fitness
         individuals.

		 <b>Model of:</b> SelectionStrategyConcept.
		 */
		struct truncation {
            //! Initializing constructor.
			template <typename Population, typename EA>
			truncation(std::size_t n, Population& src, EA& ea) {
            }

			//! Copy the n most-fit individuals to dst.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                assert(src.size() >= n);
                std::sort(src.begin(), src.end(), algorithm::fitness_comp<EA>(ea));
                typename Population::reverse_iterator rl=src.rbegin();
                std::advance(rl, n);
                dst.append(src.rbegin(), rl);
			}
		};

	} // selection
} // ealib

#endif
