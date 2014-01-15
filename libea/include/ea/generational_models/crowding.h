/* crowding.h
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
#ifndef _EA_GENERATIONAL_MODELS_CROWDING_H_
#define _EA_GENERATIONAL_MODELS_CROWDING_H_

#include <algorithm>
#include <ea/algorithm.h>
#include <ea/meta_data.h>
#include <ea/selection/proportionate.h>
#include <ea/selection/tournament.h>

namespace ealib {
	namespace generational_models {
		
		/*! Deterministic crowding.
         
         From \cite{Mahfoud1992}:
         - Randomly pair up all individuals from the population without replacement
         - Recombine each pair, producing two offspring, and mutate the offspring
         - Match offspring with the parent to which it is most similar
         - Most fit of both (parent,offspring) pairs survives
		 */
        template <typename DistanceMeasure=algorithm::hamming_distance_functor>
		struct deterministic_crowding {
            typedef DistanceMeasure distance_measure_type;
			
			//! Apply this generational model to the EA to produce a single new generation.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
                
                assert(population.size() % 2 == 0);
                distance_measure_type dmt;
                
                // random pairs of parents:
                std::random_shuffle(population.begin(), population.end(), ea.rng());
                
                Population next_gen;
                for(typename Population::iterator i=population.begin(); i!=population.end(); ) {
                    Population parents, offspring;
                    parents.push_back(*i++);
                    parents.push_back(*i++);

                    recombine(parents, offspring, typename EA::recombination_operator_type(), ea);
                    mutate(offspring.begin(), offspring.end(), ea);
                    
                    // which offspring goes w/ which parent?
                    if(dmt(*parents[0], *offspring[0], ea) > dmt(*parents[0], *offspring[1], ea)) {
                        std::swap(offspring[0], offspring[1]);
                    }
                    
                    if(ealib::fitness(*parents[0],ea) > ealib::fitness(*offspring[0],ea)) {
                        next_gen.push_back(parents[0]);
                    } else {
                        next_gen.push_back(offspring[0]);
                    }

                    if(ealib::fitness(*parents[1],ea) > ealib::fitness(*offspring[1],ea)) {
                        next_gen.push_back(parents[1]);
                    } else {
                        next_gen.push_back(offspring[1]);
                    }
                }

				// and swap it in for the current population:
                std::swap(population, next_gen);
			}
		};
		
	} // generational_models
} // ea

#endif
