/* generational.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2014 David B. Knoester.
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

#include <ea/metadata.h>
#include <ea/selection/tournament.h>
#include <ea/selection/none.h>

namespace ealib {
	namespace generational_models {
		
		/*! Generation-based generational model.
         
         This generational model selects parents from the existing population,
         recombines them to produce offspring, and then the offspring are mutated
         and replace the parents.
		 */
        template
        < typename ParentSelectionStrategy=selection::tournament< >
        , typename SurvivorSelectionStrategy=selection::none
        > struct generational {
            typedef ParentSelectionStrategy parent_selection_type;
            typedef SurvivorSelectionStrategy survivor_selection_type;
			
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
                // are there survivors?
                Population survivors;
                select<survivor_selection_type>(population, survivors, ea);
                
                // build the offspring:
                Population offspring;
                std::size_t n = population.size() - survivors.size();
                
                recombine_n(population, offspring,
                            parent_selection_type(n,population,ea),
                            typename EA::recombination_operator_type(),
                            n, ea);
                
				// mutate them:
				mutate(offspring.begin(), offspring.end(), ea);
                
                // add the survivors:
                offspring.insert(offspring.end(), survivors.begin(), survivors.end());
				
				// and swap them in for the current population:
                std::swap(population, offspring);
			}
		};
		
	} // generational_models
} // ea

#endif
