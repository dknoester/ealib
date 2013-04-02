/* qhfc.h
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
#ifndef _EA_GENERATIONAL_MODELS_QHFC_H_
#define _EA_GENERATIONAL_MODELS_QHFC_H_

#include <algorithm>
#include <ea/meta_data.h>
#include <ea/generational_model.h>

namespace ealib {
	namespace generational_models {
		
		/*! Steady-state generational model.
		 
		 This generational model defines the traditional genetic algorithm
		 crossover/mutate/select loop~\cite{eiben2007introduction}.
		 
		 Parents are selected from the current population and recombined to produce
		 offspring.  Some of the resulting offspring are then mutated.  Finally,
		 individuals from the joint population of parents and offspring are selected
		 for inclusion in the next generation.
		 */
		struct qhfc : public generational_model {
			
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));

			}
		};
		
	} // generational_models
} // ea

#endif
