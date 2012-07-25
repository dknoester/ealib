/* evolutionary_algorithm.h
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
#ifndef _EA_EVOLUTIONARY_ALGORITHM_H_
#define _EA_EVOLUTIONARY_ALGORITHM_H_

#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/attributes.h>
#include <ea/concepts.h>
#include <ea/generational_models/synchronous.h>
#include <ea/individual.h>
#include <ea/fitness_function.h>
#include <ea/initialization.h>
#include <ea/interface.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/selection.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>

namespace ea {
    
	/*! Generic evolutionary algorithm class.
	 
	 This class is designed to be generic, such that all the main features of evolutionary
	 algorithms can be easily incorporated.  The focus of this class is on the 
	 common features of most EAs, while leaving the problem-specific components 
	 easily customizable.  For example, this class includes selection strategies
	 such as tournament selection, but does not specify fitness functions.
	 */
	template <
	typename Representation,
	typename MutationOperator,
	typename FitnessFunction,
	typename RecombinationOperator=recombination::two_point_crossover,
	typename GenerationalModel=generational_models::synchronous<selection::proportionate< >, selection::tournament< > >,
	typename Initializer=initialization::complete_population<initialization::random_individual>,
    template <typename> class IndividualAttrs=individual_attributes,
    template <typename,typename,typename> class Individual=individual,
	template <typename,typename> class Population=population,
	template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ea::default_rng_type>
	class evolutionary_algorithm {
    public:
        //! This evolutionary_algorithm's type.
        typedef evolutionary_algorithm<Representation, MutationOperator, FitnessFunction,
        RecombinationOperator, GenerationalModel, Initializer, IndividualAttrs,
        Individual, Population, EventHandler, MetaData, RandomNumberGenerator
        > ea_type;
        
        //! Representation type.
        typedef Representation representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::fitness_type fitness_type;
        //! Attributes attached to individuals.
        typedef IndividualAttrs<ea_type> individual_attr_type;
        //! Individual type.
        typedef Individual<representation_type,fitness_type,individual_attr_type> individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! Mutation operator type.
        typedef MutationOperator mutation_operator_type;
        //! Crossover operator type.
        typedef RecombinationOperator recombination_operator_type;
        //! Generational model type.
        typedef GenerationalModel generational_model_type;
        //! Population type.
        typedef Population<individual_type, individual_ptr_type> population_type;
        //! Value type stored in population.
        typedef typename population_type::value_type population_entry_type;
        //! Meta-data type.
        typedef MetaData md_type;
        //! Population initializer type.
        typedef Initializer initializer_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;
        //! Event handler.
        typedef EventHandler<ea_type> event_handler_type;
        
        //! Default constructor.
        evolutionary_algorithm() {
        }
                
        //! Initialize this EA.
        void initialize() {
            _fitness_function.initialize(*this);
        }        
        
        //! Generates the initial population.
        void generate_initial_population() {
            initializer_type init;
            init(*this);
            calculate_fitness(_population.begin(), _population.end(), *this);
        }

        //! Reset the population.
        void reset() {
            nullify_fitness(_population.begin(), _population.end(), *this);
        }
        
        //! Advance the epoch of this EA by n updates.
        void advance_epoch(std::size_t n) {
            for( ; n>0; --n) {
                update();
            }
            _events.record_statistics(*this);
            _events.end_of_epoch(*this);
        }
        
        //! Advance this EA by one update.
        void update() {
            _events.record_statistics(*this);
            _generational_model(_population, *this);
            _generational_model.next_update();
            _events.end_of_update(*this);
        }
        
        //! Returns the current update of this EA.
        unsigned long current_update() {
            return _generational_model.current_update();
        }

        //! Perform any needed preselection.
        void preselect(population_type& src) {
            relativize_fitness(src.begin(), src.end(), *this);
        }
        
        //! Calculate fitness (non-stochastic).
        void evaluate_fitness(individual_type& indi) {
            indi.fitness() = _fitness_function(indi, *this);
        }
        
        //! Calculate fitness (stochastic).
        void evaluate_fitness(individual_type& indi, rng_type& rng) {
            indi.fitness() = _fitness_function(indi, rng, *this);
        }
        
        //! Accessor for the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for the population model object.
        population_type& population() { return _population; }
        
        //! Accessor for this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Accessor for the fitness function object.
        fitness_function_type& fitness_function() { return _fitness_function; }
        
        //! Accessor for the generational model object.
        generational_model_type& generational_model() { return _generational_model; }
        
        //! Returns the event handler.
        event_handler_type& events() { return _events; }
        
    protected:
        rng_type _rng; //!< Random number generator.
        fitness_function_type _fitness_function; //!< Fitness function object.
        population_type _population; //!< Population instance.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);
            ar & boost::serialization::make_nvp("meta_data", _md);
        }
    };
    
}

#endif
