/* in_place.h
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
#ifndef _EA_GENERATIONAL_MODELS_IN_PLACE_H_
#define _EA_GENERATIONAL_MODELS_IN_PLACE_H_

#include <ea/meta_data.h>
#include <ea/selection/proportionate.h>
#include <ea/selection/tournament.h>

namespace ealib {
    namespace generational_models {
		
		/*! In-place generational model.
		 
         This generational model doesn't, in fact, produce new generations.  Rather,
         it applies mutations to all individuals in the population.  This effectively
         turns the population into a "superorganism."
         
         Note that the individual mutation rates should be probably be reduced
         accordingly.
         
         Parent and survivor selection strategies are used to handle the case of a
         population that should grow or shrink, respectively.
		 */
        template <
        typename ParentSelectionStrategy=selection::proportionate< >,
        typename SurvivorSelectionStrategy=selection::tournament< > >
        struct in_place {
            typedef ParentSelectionStrategy parent_selection_type;
            typedef SurvivorSelectionStrategy survivor_selection_type;
			
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
                if(population.size() > get<POPULATION_SIZE>(ea)) {
                    // select individuals for survival:
                    Population survivors;
                    select_n<survivor_selection_type>(population, survivors,
                                                      get<POPULATION_SIZE>(ea), ea);
                    std::swap(population, survivors);
                }
                
                if(population.size() < get<POPULATION_SIZE>(ea)) {
                    // recombine the survivors to produce offspring:
                    unsigned int n = get<POPULATION_SIZE>(ea) - population.size();
                    Population offspring;
                    recombine_n(population, offspring,
                                parent_selection_type(n,population,ea),
                                typename EA::recombination_operator_type(),
                                n, ea);
                    population.insert(population.end(), offspring.begin(), offspring.end());
                }
                
                // mutate everyone in the population:
				mutate(population.begin(), population.end(), ea);
			}
		};
		
	} // generational_models
} // ea

#endif
