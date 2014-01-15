/* generational.h
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
#ifndef _EA_GENERATIONAL_MODELS_GENERATIONAL_H_
#define _EA_GENERATIONAL_MODELS_GENERATIONAL_H_

#include <ea/meta_data.h>
#include <ea/selection/proportionate.h>
#include <ea/selection/tournament.h>

namespace ealib {

    LIBEA_MD_DECL(GENERATIONAL_REPLACEMENT_RATE_P, "ea.generational_model.generational.replacement_rate.p", double);

	namespace generational_models {
		
		/*! Generational model.
		 
		 This generational model defines the traditional genetic algorithm
		 crossover/mutate/select loop~\cite{eiben2007introduction}.
		 
		 Parents are selected from the current population and recombined to produce
		 offspring.  Some of the resulting offspring are then mutated.  Finally, 
		 individuals from the joint population of parents and offspring are selected
		 for inclusion in the next generation.
		 */
        template <
        typename ParentSelectionStrategy=selection::proportionate< >,
        typename SurvivorSelectionStrategy=selection::tournament< > >
		struct generational {
            typedef ParentSelectionStrategy parent_selection_type;
            typedef SurvivorSelectionStrategy survivor_selection_type;
			
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
                
                // build the offspring:
                Population offspring;
                std::size_t n = static_cast<std::size_t>(get<GENERATIONAL_REPLACEMENT_RATE_P>(ea)*population.size());
                
                recombine_n(population, offspring,
                            parent_selection_type(n,population,ea),
                            typename EA::recombination_operator_type(),
                            n, ea);
                
				// mutate them:
				mutate(offspring.begin(), offspring.end(), ea);
				
				// add the offspring to the current population:
				population.insert(population.end(), offspring.begin(), offspring.end());
                
				// select individuals for survival:
				Population next_gen;
                select_n<survivor_selection_type>(population, next_gen, get<POPULATION_SIZE>(ea), ea);
				
				// and swap it in for the current population:
                std::swap(population, next_gen);
			}
		};
		
	} // generational_models
} // ea

#endif
