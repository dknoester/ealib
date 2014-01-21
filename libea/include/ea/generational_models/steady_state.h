/* steady_state.h
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
#ifndef _EA_GENERATIONAL_MODELS_STEADY_STATE_H_
#define _EA_GENERATIONAL_MODELS_STEADY_STATE_H_

#include <ea/meta_data.h>
#include <ea/selection/proportionate.h>
#include <ea/selection/tournament.h>

namespace ealib {

    LIBEA_MD_DECL(STEADY_STATE_LAMBDA, "ea.generational_model.steady_state.lambda", unsigned int);

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
		struct steady_state {
            typedef ParentSelectionStrategy parent_selection_type;
            typedef SurvivorSelectionStrategy survivor_selection_type;
			
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
                // how many survivors?
                unsigned int n = get<POPULATION_SIZE>(ea) - get<STEADY_STATE_LAMBDA>(ea);
                
                // select individuals for survival:
				Population survivors;
                select_n<survivor_selection_type>(population, survivors, n, ea);
                
                // recombine the survivors to produce offspring:
                Population offspring;
                recombine_n(survivors, offspring,
                            parent_selection_type(n,survivors,ea),
                            typename EA::recombination_operator_type(),
                            get<STEADY_STATE_LAMBDA>(ea), ea);
                
                // mutate them:
				mutate(offspring.begin(), offspring.end(), ea);
				
				// add the offspring to the list of survivors:
				survivors.insert(survivors.end(), offspring.begin(), offspring.end());
                
				// and swap 'em in for the current population:
                std::swap(population, survivors);
			}
		};
		
	} // generational_models
} // ea

#endif
