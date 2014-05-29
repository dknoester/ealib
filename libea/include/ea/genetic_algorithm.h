/* genetic_algorithm.h
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
#ifndef _EA_GENETIC_ALGORITHM_H_
#define _EA_GENETIC_ALGORITHM_H_

#include <ea/evolutionary_algorithm.h>
#include <ea/stop.h>
#include <ea/configuration.h>
#include <ea/ancestors.h>

namespace ealib {
    
	/*! Genetic algorithm.
     
     This class specializes evolutionary_algorithm to provide a canonical
     genetic algorithm.  It is designed such that the commonly changed pieces 
     can be easily swapped out.  If more advanced control over the features of 
     the GA are needed, the reader is referred to evolutionary_algorithm.h.
	 */
//	template
//    < typename Representation
//    , typename FitnessFunction
//    , typename AncestorGenerator
//	, typename MutationOperator
//	, typename RecombinationOperator
//	, typename GenerationalModel
//    , typename EarlyStopCondition=dont_stop
//    , typename UserDefinedConfiguration=default_configuration
//    , typename PopulationGenerator=fill_population
//    > class genetic_algorithm
//    : public evolutionary_algorithm
//    < individual<Representation, FitnessFunction>
//    , AncestorGenerator
//    , MutationOperator
//    , RecombinationOperator
//    , GenerationalModel
//    , EarlyStopCondition
//    , UserDefinedConfiguration
//    , PopulationGenerator
//    > {
//    };
    
} // ealib

#endif
