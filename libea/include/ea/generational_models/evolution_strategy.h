/* evolution_strategy.h
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
#ifndef _EA_GENERATIONAL_MODELS_EVOLUTION_STRATEGY_H_
#define _EA_GENERATIONAL_MODELS_EVOLUTION_STRATEGY_H_

#include <ea/access.h>
#include <ea/generational_models/generational.h>
#include <ea/selection/rank.h>

namespace ealib {

	LIBEA_MD_DECL(ES_LAMBDA, "ea.generational_model.evolution_strategy.lambda", unsigned int);
	LIBEA_MD_DECL(ES_MU, "ea.generational_model.evolution_strategy.mu", unsigned int);
	LIBEA_MD_DECL(ES_RHO, "ea.generational_model.evolution_strategy.rho", unsigned int);
	
	namespace generational_models {
		
		/*! Evolution strategy generational model.
		 
		 This generational model reflects the "traditional" \lambda, \mu, \rho, 
		 comma, and plus ES.
		 
		 +: Mutant is preferred only if its fitness is >= to its parent's.
		 ,: Best mutant(s) are preferred regardless of parent's fitness.
		 
		 The general form of an ES is: (\mu / \rho [+,], \lambda), where:
		 
		 \mu: Parent population size
		 \rho: Recombinant population size
		 \lamba: Mutant population size
		 
		 The canonical ES is (1+1).  Interestingly, selection is all via a rank-
		 ordering of individuals.
		 */
		struct commaS { };
		struct plusS { };
		
		template
		< typename SurvivorTag=commaS
		, typename RankSelectionStrategy=selection::rank<access::fitness>
		> struct evolution_strategy {
			typedef RankSelectionStrategy rank_selection_type;
			typedef SurvivorTag survivor_tag_type;
			
			//! Perform comma replacement.
			template <typename Population, typename EA>
			void replace(Population& population, Population& mutants, Population& survivors, commaS, EA& ea) {
				select_n<rank_selection_type>(mutants, survivors, get<ES_MU>(ea), ea);
			}
			
			/*! Perform plus replacement.
			 
			 Note: we don't make an effort to distinguish between parents and offspring
			 with equivalent fitness.
			 */
			template <typename Population, typename EA>
			void replace(Population& population, Population& mutants, Population& survivors, plusS, EA& ea) {
				Population offspring(population);
				offspring.insert(offspring.end(), mutants.begin(), mutants.end());
				select_n<rank_selection_type>(offspring, survivors, get<ES_MU>(ea), ea);
			}
			
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {

				// select and generate the mutants:
				Population mutants;
				select_n<rank_selection_type>(population, mutants, get<ES_LAMBDA>(ea), ea);
				mutate(mutants.begin(), mutants.end(), ea);
				
				// select and generate the recombinants:
				if(get<ES_RHO>(ea) > 0) {
					recombine_n(population, mutants,
								parent_selection_type(get<ES_RHO>(ea),population,ea),
								typename EA::recombination_operator_type(),
								get<ES_RHO>(ea), ea);
				}
				
				// comma or plus?
				Population survivors;
				replace(population, mutants, survivors, survivor_tag_type(), ea);
				
				// and swap 'em in for the current population:
				std::swap(population, survivors);
			}
		};
	
	} // generational_models
} // ea

#endif
