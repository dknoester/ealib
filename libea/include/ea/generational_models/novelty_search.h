#ifndef _EA_GENERATIONAL_MODELS_NOVELTY_SEARCH_H_
#define _EA_GENERATIONAL_MODELS_NOVELTY_SEARCH_H_

#include <algorithm>
#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/generational_model.h>
#include <ea/selection/fitness_proportional.h>
#include <ea/selection/tournament.h>

namespace ea {
	namespace generational_models {
		
        LIBEA_MD_DECL(NOVELTY_THRESHOLD, "ea.generational_model.novelty_search.threshold", unsigned int);
        
        template <typename Population>
		struct novelty_search : public generational_model {
            typedef Population population_type;
            typedef ParentSelectionStrategy parent_selection_type;
            
            
            //! Novelty-based FPS.
            struct novelty_fps {
                template <typename Population, typename EA>
                novelty_fps(std::size_t n, Population& src, EA& ea) { 
                    _sum = 0.0;
                    for(typename Population::iterator i=src.begin(); i!=src.end(); ++i) {
                        _sum += novelty(i);
                    }
                }
                
                template <typename Population, typename EA>
                void operator()(Population& src, Population& dst, std::size_t n, EA& ea) {
                    assert(_sum > 0.0);
                    typename EA::rng_type::real_rng_type my_rng = ea.rng().uniform_real_rng(0.0, _sum);
                    
                    for( ; n>0; n--) {
                        // need to base this on novelty!
//                        dst.append(*algorithm::roulette_wheel(my_rng(), src.begin(), src.end(), ea));
                    }
                }
            };
            
                       
            //! Sort by novelty in DESCENDING order.
            struct novelty_comparator {
                template <typename IndividualPtr>
                bool operator()(IndividualPtr a, IndividualPtr b) {
                    return novelty(a) > novelty(b);
                }
            };
            
            //! Recalculates novelty of the current population relative to the archive.
            template <typename Population, typename EA>
            void recalculate_novelty(typename Population::iterator f, typename Population::iterator l, EA& ea) {
                for( ; f!=l; ++f) {
                    for(typename Population::iterator i=(f+1); i!=l; ++i)
                        novelty(f) = std::min(novelty(f), dis(f,i));
                    }
                    for(typename Population::iterator i=_archive.begin(); i!=_archive.end(); ++i) {
                        novelty(f) = std::min(novelty(f), dis(f,i));
                    }
                }
            }            
            
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
				
                // build the offspring:
                Population offspring;
                std::size_t n = static_cast<std::size_t>(get<REPLACEMENT_RATE_P>(ea)*population.size());
                
                recombine_n(population, offspring,
                            novelty_fps(N,parents,ea),
                            typename EA::recombination_operator_type(),
                            n, ea);
                
				// mutate them:
				mutate(offspring.begin(), offspring.end(), ea);
                
                // make sure offspring have their fitness calculated:
                calculate_fitness(offspring.begin(), offspring.end(), ea);

                // add the offspring to the population:
                population.append(offspring.begin(), offspring.end(), ea);
                
                // recalculate novelty - this is dynamic, and relative to the archive and population:
                recalculate_novelty(population.begin(), population.end(), ea);
                
                // add offspring to the archive:
                for(typename Population::iterator i=offspring.begin(); i!=offspring.end(); ++i) {
                    if(novelty(ind(i,ea)) > get<NOVELTY_THRESHOLD>(ea)) {
                        _archive.append(i);
                    }
                }
                
                // truncate the least novel:
                std::sort(population.begin(), population.end(), novelty_comparator());
                population.resize(get<POPULATION_SIZE>(ea));
			}
            
            population_type _archive; //!< Archive of novel individuals.
		};
		
	} // generational_models
} // ea

#endif
