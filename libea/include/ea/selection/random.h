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
            
            // death (by virtue of not being copied into the next generation).
            // this can be done in a few different ways -- however, the only
            // correct way is one that ensures that each individual is tested
            // for death *once*, and that the selection appears to occur in
            // parallel:
            //                Population survivors;
            //                for(typename Population::iterator i=population.begin(); i!=population.end(); ++i) {
            //                    if(ea.rng().p(1.0-get<REPLACEMENT_RATE_P>(ea))) {
            //                        survivors.append(i);
            //                    }
            //                }
            //                std::swap(population, survivors);

            
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
