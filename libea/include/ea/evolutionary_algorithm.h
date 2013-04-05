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

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/attributes.h>
#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/generational_models/steady_state.h>
#include <ea/individual.h>
#include <ea/fitness_function.h>
#include <ea/ancestors.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/selection.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>

namespace ealib {
    
	/*! Generic evolutionary algorithm class.
	 
	 This class is designed to be generic, such that all the main features of evolutionary
	 algorithms can be easily incorporated.  The focus of this class is on the 
	 common features of most EAs, while leaving the problem-specific components 
	 easily customizable.
	 */
	template <
	typename Representation,
	typename MutationOperator,
	typename FitnessFunction,
    template <typename> class ConfigurationStrategy=abstract_configuration,
	typename RecombinationOperator=recombination::two_point_crossover,
	typename GenerationalModel=generational_models::steady_state<selection::proportionate< >, selection::tournament< > >,
    template <typename> class IndividualAttrs=default_attributes,
    template <typename> class Individual=individual,
	template <typename,typename> class Population=population,
	template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ealib::default_rng_type>
	class evolutionary_algorithm {
    public:        
        //! Configuration object type.
        typedef ConfigurationStrategy<evolutionary_algorithm> configuration_type;
        //! Representation type.
        typedef Representation representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::fitness_type fitness_type;
        //! Attributes attached to individuals.
        typedef IndividualAttrs<evolutionary_algorithm> individual_attr_type;
        //! Individual type.
        typedef Individual<evolutionary_algorithm> individual_type;
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
        //! Meta-data type.
        typedef MetaData md_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;
        //! Event handler.
        typedef EventHandler<evolutionary_algorithm> event_handler_type;
        //! Iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        //! Reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;

        //! Default constructor.
        evolutionary_algorithm() {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<evolutionary_algorithm>));
            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
        }
        
        //! Configure this EA.
        void configure() {
            _configurator.configure(*this);
        }
        
        //! Build the initial population.
        void initial_population() {
            _configurator.initial_population(*this);
        }
                
        //! Initialize this EA.
        void initialize() {
            initialize_fitness_function(_fitness_function, *this);
            _configurator.initialize(*this);
        }
        
        //! Reset the population.
        void reset() {
            nullify_fitness(_population.begin(), _population.end(), *this);
            _configurator.reset(*this);
        }
        
        //! Remove all individuals in this EA.
        void clear() {
            _population.clear();
        }
        
        //! Begin an epoch.
        void begin_epoch() {
            _events.record_statistics(*this);
        }
        
        //! End an epoch.
        void end_epoch() {
            _events.end_of_epoch(*this);
        }

        //! Advance this EA by one update.
        void update() {
            if(!_population.empty()) {
                _generational_model(_population, *this);
            }
            _events.end_of_update(*this);

            _generational_model.next_update();
            _events.record_statistics(*this);
        }
        
        //! Build an individual from the given representation.
        individual_ptr_type make_individual(const representation_type& r) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Build a copy of an individual.
        individual_ptr_type make_individual(const individual_type& r) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Append individual x to the population.
        void append(individual_ptr_type x) {
            _population.insert(_population.end(), x);
        }
        
        //! Append the range of individuals [f,l) to the population.
        template <typename ForwardIterator>
        void append(ForwardIterator f, ForwardIterator l) {
            _population.insert(_population.end(), f, l);
        }
        
        //! Erase the given individual from the population.
        void erase(iterator i) {
            _population.erase(i.base());
        }

        //! Erase the given range from the population.
        void erase(iterator f, iterator l) {
            _population.erase(f.base(), l.base());
        }
        
        //! Accessor for the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Accessor for the fitness function object.
        fitness_function_type& fitness_function() { return _fitness_function; }
        
        //! Accessor for the generational model object.
        generational_model_type& generational_model() { return _generational_model; }
        
        //! Returns the current update of this EA.
        unsigned long current_update() { return _generational_model.current_update(); }
        
        //! Returns the event handler.
        event_handler_type& events() { return _events; }

        //! Returns the configuration object.
        configuration_type& configuration() { return _configurator; }
        
        //! Accessor for the population model object.
        population_type& population() { return _population; }
        
        //! Return the number of individuals in this EA.
        std::size_t size() const {
            return _population.size();
        }

        //! Return the n'th individual in the population.
        individual_type& operator[](std::size_t n) {
            return *_population[n];
        }
        
        //! Returns a begin iterator to the population.
        iterator begin() {
            return iterator(_population.begin());
        }
        
        //! Returns an end iterator to the population.
        iterator end() {
            return iterator(_population.end());
        }
        
        //! Returns a begin iterator to the population (const-qualified).
        const_iterator begin() const {
            return const_iterator(_population.begin());
        }
        
        //! Returns an end iterator to the population (const-qualified).
        const_iterator end() const {
            return const_iterator(_population.end());
        }
        
        //! Returns a reverse begin iterator to the population.
        reverse_iterator rbegin() {
            return reverse_iterator(_population.rbegin());
        }
        
        //! Returns a reverse end iterator to the population.
        reverse_iterator rend() {
            return reverse_iterator(_population.rend());
        }
        
        //! Returns a reverse begin iterator to the population (const-qualified).
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(_population.rbegin());
        }
        
        //! Returns a reverse end iterator to the population (const-qualified).
        const_reverse_iterator rend() const {
            return const_reverse_iterator(_population.rend());
        }

    protected:
        rng_type _rng; //!< Random number generator.
        fitness_function_type _fitness_function; //!< Fitness function object.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configurator; //!< Configuration object.
        population_type _population; //!< Population instance.

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
    
} // ea

#endif
