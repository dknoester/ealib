/* death_birth_process.h
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
#ifndef _EA_GENERATIONAL_MODELS_DEATH_GROWTH_PROCESS_H_
#define _EA_GENERATIONAL_MODELS_DEATH_GROWTH_PROCESS_H_

#include <algorithm>
#include <cmath>
#include <ea/interface.h>
#include <ea/generational_model.h>
#include <ea/selection/proportionate.h>
#include <ea/selection/random.h>

namespace ea {
    
	namespace generational_models {
		
        /*! Death-birth process generational model.

		 The idea here is that all individuals that are slated to die (according
         to a configurable replacement rate) die at once, and then the population
         expands back to that size via fitness proportionate selection (with replacement).
         
         This is a reasonable approximation of the Moran process, as described by Patrick Moran.
         
         \warning Fitness can not be negative.
		 */
        template <typename SurvivorSelectionStrategy=selection::proportionate< > >
		struct death_birth_process : public generational_model {
            typedef SurvivorSelectionStrategy survivor_selection_type;
            
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
                            survivor_selection_type(n,survivors,ea),
                            typename EA::recombination_operator_type(),
                            n, ea);
                
                // mutate them:
				mutate(offspring.begin(), offspring.end(), ea);
				
                // calculate fitness:
                calculate_fitness(offspring.begin(), offspring.end(), ea);
                
				// add the offspring to the list of survivors:
				survivors.insert(survivors.end(), offspring.begin(), offspring.end());
                
				// and swap 'em in for the current population:
                std::swap(population, survivors);
            }
		};
		
	} // generational_models
} // ea

#endif
