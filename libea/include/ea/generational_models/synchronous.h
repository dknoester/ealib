#ifndef _EA_GENERATIONAL_MODELS_SYNCHRONOUS_H_
#define _EA_GENERATIONAL_MODELS_SYNCHRONOUS_H_

#include <algorithm>
#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/generational_model.h>
#include <ea/selection/fitness_proportional.h>
#include <ea/selection/tournament.h>

namespace ea {
	namespace generational_models {
		
		/*! Synchronous generational model.
		 
		 This generational model defines the traditional genetic algorithm
		 crossover/mutate/select loop~\cite{eiben2007introduction}.
		 
		 Parents are selected from the current population and recombined to produce
		 offspring.  Some of the resulting offspring are then mutated.  Finally, 
		 individuals from the joint population of parents and offspring are selected
		 for inclusion in the next generation.
		 */
        template <
        typename ParentSelectionStrategy=selection::fitness_proportional,
        typename SurvivorSelectionStrategy=selection::tournament>
		struct synchronous : public generational_model {
            typedef ParentSelectionStrategy parent_selection_type;
            typedef SurvivorSelectionStrategy survivor_selection_type;
			
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
				
                // build the offspring:
                Population offspring;
                std::size_t n = static_cast<std::size_t>(get<REPLACEMENT_RATE_P>(ea)*population.size());
                
                recombine_n(population, offspring,
                            parent_selection_type(n,population,ea),
                            typename EA::recombination_operator_type(),
                            n, ea);
                
				// mutate them:
				mutate(offspring.begin(), offspring.end(), ea);
				
				// add the offspring to the current population:
				population.append(offspring.begin(), offspring.end());
                
                // make sure all have their fitness calculated:
                calculate_fitness(population.begin(), population.end(), ea);
                
				// select individuals for survival to the next generation:into the next generation:
				Population next_gen;
                select_n<survivor_selection_type>(population, next_gen, get<POPULATION_SIZE>(ea), ea);
				
				// and swap it in for the current population:
                std::swap(population, next_gen);
			}
		};
		
	} // generational_models
} // ea

#endif
