#ifndef _EA_GENERATIONAL_MODELS_DEATH_GROWTH_PROCESS_H_
#define _EA_GENERATIONAL_MODELS_DEATH_GROWTH_PROCESS_H_

#include <algorithm>
#include <cmath>
#include <ea/interface.h>
#include <ea/generational_model.h>
#include <ea/selection/proportional.h>
#include <ea/selection/random.h>

namespace ea {
    
	namespace generational_models {
		
        struct no_growth_tag { };
        struct exponential_growth_tag { };
        
		/*! Death-birth process generational model.

		 The idea here is that all individuals that are slated to die (according
         to a configurable replacement rate) die at once, and then the population
         expands back to that size via fitness proportional selection (with replacement).
         
         This is a reasonable approximation of the Moran process, as described by Patrick Moran.
         
         \warning Fitness can not be negative.
		 */
        template <typename GrowthType=no_growth_tag>
		struct death_birth_process : public generational_model {
            typedef GrowthType growth_type;
            
            double adjust_fitness_sum(double fsum, double f, no_growth_tag) {
                return fsum;
            }

            double adjust_fitness_sum(double fsum, double f, exponential_growth_tag) {
                return fsum + f;
            }
            
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));				
                
                // how many survivors?
                std::size_t n = static_cast<std::size_t>((1.0 - get<REPLACEMENT_RATE_P>(ea)) * get<POPULATION_SIZE>(ea));
                
                // select individuals for survival:
				Population survivors;
                select_n<selection::random>(population, survivors, n, ea);
                
                // how many offspring?
                n = get<POPULATION_SIZE>(ea) - survivors.size();
				
                // recombine the survivors to produce offspring:
                Population offspring;
                recombine_n(survivors, offspring,
                            selection::proportional< >(n,survivors,ea),
                            typename EA::recombination_operator_type(),
                            n, ea);
                
                // mutate them:
				mutate(offspring.begin(), offspring.end(), ea);
				
                // calculate fitness:
                calculate_fitness(offspring.begin(), offspring.end(), ea);
                
				// add the offspring to the list of survivors:
				survivors.append(offspring.begin(), offspring.end());
                
				// and swap 'em in for the current population:
                std::swap(population, survivors);
            }
		};
		
	} // generational_models
} // ea

#endif
