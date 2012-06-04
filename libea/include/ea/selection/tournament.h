#ifndef _EA_SELECTION_TOURNAMENT_H_
#define _EA_SELECTION_TOURNAMENT_H_

#include <algorithm>
#include <iterator>
#include <ea/algorithm.h>
#include <ea/meta_data.h>

namespace ea {
	namespace selection {
		
		/*! Tournament selection.
		 
		 This selection method runs tournaments of size N and selects the K individuals
		 with greatest fitness.
		 
		 <b>Model of:</b> SelectionStrategyConcept.
		 */
		struct tournament {
            //! Initializing constructor.
			template <typename Population, typename EA>
			tournament(std::size_t n, Population& src, EA& ea) { 
            }

			//! Select n individuals via tournament selection.
			template <typename Population, typename EA>
			void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
				std::size_t N = get<TOURNAMENT_SELECTION_N>(ea);
				std::size_t K = get<TOURNAMENT_SELECTION_K>(ea);
				while(n > 0) {
					Population tourney;
					std::insert_iterator<Population> tii(tourney,tourney.end());
					ea.rng().sample_without_replacement(src.begin(), src.end(), tii, N);
					
                    std::sort(tourney.begin(), tourney.end(), algorithm::fitness_comp<EA>(ea));
                    typename Population::reverse_iterator rl=tourney.rbegin();
                    std::size_t copy_size = std::min(n,K);
                    std::advance(rl, copy_size);
                    dst.append(tourney.rbegin(), rl);
					n -= copy_size;
				}
			}
		};
        
	} // selection
} // ealib

#endif
