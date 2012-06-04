#ifndef _EA_SELECTION_RANDOM_H_
#define _EA_SELECTION_RANDOM_H_

#include <iterator>

namespace ea {	
	namespace selection {
		
		/*! Random selection.
		 
		 This selection method selects individuals randomly with replacement (that is,
		 the same individual may be selected more than once.)
		 
		 <b>Model of:</b> SelectionStrategyConcept.
		 */
		struct random {
            //! Initializing constructor.
			template <typename Population, typename EA>
			random(std::size_t n, Population& src, EA& ea) {
			}
            
			//! Select n individuals at random.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
				std::insert_iterator<Population> ii(dst,dst.end());
				ea.rng().sample_with_replacement(src.begin(), src.end(), ii, n);
			}
		};
        
	} // selection
} // ealib

#endif
