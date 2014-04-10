/* markov_evolution_algorithm.h
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
#ifndef _EA_MKV_MARKOV_EVOLUTION_ALGORITHM_H_
#define _EA_MKV_MARKOV_EVOLUTION_ALGORITHM_H_

#include <ea/evolutionary_algorithm.h>
#include <ea/stop.h>
#include <ea/ancestors.h>
#include <ea/mkv/markov_network.h>

namespace ealib {
    
	/*! Markov network evolutionary algorithm.
     
     This class specializes evolutionary_algorithm to provide an algorithm specific
     to evolving Markov networks.  If more advanced control over the features of
     the GA are needed, the reader is referred to evolutionary_algorithm.h.
	 */
	template
    < typename FitnessFunction
	, typename RecombinationOperator
	, typename GenerationalModel
    , typename EarlyStopCondition=dont_stop
    , typename UserDefinedConfiguration=mkv::configuration
    , typename PopulationGenerator=fill_population
    > class markov_evolution_algorithm
    : public evolutionary_algorithm
    < individual<mkv::representation_type, FitnessFunction, markov_network< >, indirectS, mkv::default_traits>
    , mkv::ancestor_generator
    , mkv::mutation_type
    , RecombinationOperator
    , GenerationalModel
    , EarlyStopCondition
    , UserDefinedConfiguration
    , PopulationGenerator
    > {
    };

    /*! Markov network evolutionary algorithm.
     
     This class specializes evolutionary_algorithm to provide an algorithm specific
     to evolving Markov networks.  If more advanced control over the features of
     the GA are needed, the reader is referred to evolutionary_algorithm.h.
	 */
	template
    < typename FitnessFunction
	, typename RecombinationOperator
	, typename GenerationalModel
    , typename EarlyStopCondition=dont_stop
    , typename UserDefinedConfiguration=mkv::configuration
    , typename PopulationGenerator=fill_population
    > class markov_evolution_lod_algorithm
    : public evolutionary_algorithm
    < individual<mkv::representation_type, FitnessFunction, markov_network< >, indirectS, mkv::lod_default_traits>
    , mkv::ancestor_generator
    , mkv::mutation_type
    , RecombinationOperator
    , GenerationalModel
    , EarlyStopCondition
    , UserDefinedConfiguration
    , PopulationGenerator
    > {
    };
    
} // ealib

#endif
