/* metapopulation.h
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
#ifndef _EA_METAPOPULATION_H_
#define _EA_METAPOPULATION_H_

#include <ea/ancestors.h>
#include <ea/configuration.h>
#include <ea/evolutionary_algorithm.h>
#include <ea/generational_models/isolated_subpopulations.h>
#include <ea/fitness_functions/constant.h>
#include <ea/mutation.h>
#include <ea/recombination.h>
#include <ea/stop.h>
#include <ea/subpopulation.h>


namespace ealib {
        
    /*! Metapopulation evolutionary algorithm, where individuals in the population
     are themselves evolutionary algorithms.
     
     By default, a meta-population EA provides something akin to an island model,
     where the subpopulations are completely isolated from one another, and individuals
     do not migrate among subpopulations.
     
     To use a metapopulation, declare a single population EA, and then wrap it
     in a subpopulation, like so:

     typedef metapopulation<subpopulation<all_ones_ea,constant> > ea_type;

     */
    template
    < typename Subpopulation
    , typename AncestorGenerator=ancestors::default_representation
    , typename MutationOperator=mutation::operators::no_mutation
    , typename RecombinationOperator=recombination::no_recombination
    , typename GenerationalModel=generational_models::isolated_subpopulations
    , typename EarlyStopCondition=dont_stop
    , typename UserDefinedConfiguration=default_configuration
    , typename PopulationGenerator=fill_metapopulation
    > class metapopulation
    : public evolutionary_algorithm
    < Subpopulation
    , AncestorGenerator
    , MutationOperator
    , RecombinationOperator
    , GenerationalModel
    , EarlyStopCondition
    , UserDefinedConfiguration
    , PopulationGenerator
    > {
    public:
        //! Tag indicating the structure of this population.
        typedef multiPopulationS population_structure_tag;
        //! Parent type.
        typedef evolutionary_algorithm
        < Subpopulation
        , AncestorGenerator
        , MutationOperator
        , RecombinationOperator
        , GenerationalModel
        , EarlyStopCondition
        , UserDefinedConfiguration
        , PopulationGenerator
        > parent;
        //! Individual type (note that this is a *subpopulation*).
        typedef typename parent::individual_type individual_type;
        //! Individual pointer type.
        typedef typename parent::individual_ptr_type individual_ptr_type;
        //! Representation type.
        typedef typename parent::representation_type representation_type;
        //! Encoding type.
        typedef typename parent::encoding_type encoding_type;
        //! Phenotype type.
        typedef typename parent::phenotype_type phenotype_type;
        //! Phenotype pointer type.
        typedef typename parent::phenotype_ptr_type phenotype_ptr_type;
        //! Fitness function type.
        typedef typename parent::fitness_function_type fitness_function_type;
        //! Ancestor generator type.
        typedef typename parent::ancestor_generator_type ancestor_generator_type;
        //! Mutation operator type.
        typedef typename parent::mutation_operator_type mutation_operator_type;
        //! Crossover operator type.
        typedef typename parent::recombination_operator_type recombination_operator_type;
        //! Generational model type.
        typedef typename parent::generational_model_type generational_model_type;
        //! Function that checks for an early stopping condition.
        typedef typename parent::stop_condition_type stop_condition_type;
        //! User-defined configuration methods type.
        typedef typename parent::configuration_type configuration_type;
        //! Population generator type.
        typedef typename parent::population_generator_type population_generator_type;
        //! Meta-data type.
        typedef typename parent::md_type md_type;
        //! Random number generator type.
        typedef typename parent::rng_type rng_type;
        //! Event handler.
        typedef typename parent::event_handler_type event_handler_type;
        //! Population type.
        typedef typename parent::population_type population_type;
        //! Iterator over the subpopulations in this metapopulation.
        typedef typename parent::iterator iterator;
        //! Const iterator over this EA's population.
        typedef typename parent::const_iterator const_iterator;
        //! Reverse iterator over this EA's population.
        typedef typename parent::reverse_iterator reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef typename parent::const_reverse_iterator const_reverse_iterator;
        //! Type of the embedded EA.
        typedef typename individual_type::ea_type subea_type;
        //! Subpopulation type.
        typedef typename individual_type::ea_type::population_type subpopulation_type;
        
        //! Initializes this EA and any existing subpopulations.
        virtual void initialize() {
            for(iterator i=parent::begin(); i!=parent::end(); ++i) {
                i->ea().initialize();
            }
            initialize_fitness_function(parent::_fitness_function, *this);
            parent::_configuration.initialize(*this);
        }

        //! Begin an epoch.
        virtual void begin_epoch() {
            for(iterator i=parent::begin(); i!=parent::end(); ++i) {
                i->ea().begin_epoch();
            }
            parent::_events.record_statistics(*this);
        }

        //! End an epoch.
        virtual void end_epoch() {
            for(iterator i=parent::begin(); i!=parent::end(); ++i) {
                i->ea().events().end_of_epoch(i->ea()); // don't checkpoint!
            }
            parent::_events.end_of_epoch(*this); // checkpoint!
        }
        
        //! Returns a new individual built from the given representation.
        virtual individual_ptr_type make_individual(const representation_type& r=representation_type()) {
            individual_ptr_type p(new individual_type(r));
            p->ea().md() += parent::md(); // WARNING: Meta-data comes from the meta-population.
            p->ea().reset(parent::_rng.seed());
            p->ea().initialize();
            return p;
        }
        
        //! Returns a copy of an individual.
        virtual individual_ptr_type copy_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
            p->ea().md() += ind.md(); // WARNING: Meta-data comes from the individual.
            p->ea().reset(parent::_rng.seed());
            p->ea().initialize();
            return p;
        }
    };
    
}

#endif
