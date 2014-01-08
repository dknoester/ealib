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

#include <ea/individual.h>

#include <ea/concepts.h>
#include <ea/generational_models/steady_state.h>
#include <ea/individual.h>
#include <ea/phenotype.h>
#include <ea/fitness_function.h>
#include <ea/ancestors.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/selection.h>
#include <ea/stop.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>
#include <ea/configuration.h>

namespace ealib {
    
	/*! Generic evolutionary algorithm class.
	 
	 This class is designed to be generic, such that all the main features of evolutionary
	 algorithms can be easily incorporated.  The focus of this class is on the
	 common features of most EAs, while leaving the problem-specific components
	 easily customizable.
	 */
	template
    < typename Individual
    , typename AncestorGenerator
	, typename MutationOperator
	, typename RecombinationOperator
	, typename GenerationalModel
    , typename EarlyStopCondition=dont_stop
    , typename Configuration=default_configuration
    > class evolutionary_algorithm {
    public:
        //! Tag for the population structure of this evolutionary algorithm.
        typedef singlePopulationS population_structure_tag;
        //! Individual type.
        typedef Individual individual_type;
        //! Individual pointer type.
        typedef typename individual_type::individual_ptr_type individual_ptr_type;
        //! Representation type.
        typedef typename individual_type::representation_type representation_type;
        //! Encoding type.
        typedef typename individual_type::encoding_type encoding_type;
        //! Phenotype type.
        typedef typename individual_type::phenotype_type phenotype_type;
        //! Phenotype pointer type.
        typedef typename individual_type::phenotype_ptr_type phenotype_ptr_type;
        //! Fitness function type.
        typedef typename individual_type::fitness_function_type fitness_function_type;
        //! Ancestor generator type.
        typedef AncestorGenerator ancestor_generator_type;
        //! Mutation operator type.
        typedef MutationOperator mutation_operator_type;
        //! Crossover operator type.
        typedef RecombinationOperator recombination_operator_type;
        //! Generational model type.
        typedef GenerationalModel generational_model_type;
        //! Function that checks for an early stopping condition.
        typedef EarlyStopCondition stop_condition_type;
        //! Configuration methods type.
        typedef Configuration configuration_type;
        //! Meta-data type.
        typedef meta_data md_type;
        //! Random number generator type.
        typedef default_rng_type rng_type;
        //! Event handler.
        typedef event_handler<evolutionary_algorithm> event_handler_type;
        //! Population type.
        typedef population<individual_type,individual_ptr_type> population_type;
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
            configure();
        }
        
        //! Copy constructor (note that this is *not* a complete copy).
        evolutionary_algorithm(const evolutionary_algorithm& that) {
            _rng = that._rng;
            _fitness_function = that._fitness_function;
            _md = that._md;
            // gm doesn't copy...
            // events doesn't copy...
            // configurator doesn't copy...
            // copy individuals:
            for(const_iterator i=that.begin(); i!=that.end(); ++i) {
                individual_ptr_type q = make_individual(*i);
                append(q);
            }
            configure();
        }
        
        //! Configure this EA.
        void configure() {
            _configuration.configure(*this);
        }
        
        //! Build the initial population.
        void initial_population() {
            generate_ancestors(ancestor_generator_type(), get<POPULATION_SIZE>(*this)-_population.size(), *this);
        }
        
        //! Initialize this EA.
        void initialize() {
            initialize_fitness_function(_fitness_function, *this);
            _configuration.initialize(*this);
        }
        
        //! Reset the population.
        void reset() {
            nullify_fitness(_population.begin(), _population.end(), *this);
            _configuration.reset(*this);
        }
        
        //! Reset the RNG.
        void reset_rng(unsigned int s) {
            put<RNG_SEED>(s,*this); // save the seed!
            _rng.reset(s);
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
        
        //! Returns trus if this EA should be stopped.
        bool stop() {
            return _stop(*this);
        }
        
        //! Build an individual from the given representation.
        individual_ptr_type make_individual(const representation_type& r=representation_type()) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Build a copy of an individual.
        individual_ptr_type make_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
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
        
        //! Accessor for this EA's meta-data (const-qualified).
        const md_type& md() const { return _md; }
        
        //! Accessor for the fitness function object.
        fitness_function_type& fitness_function() { return _fitness_function; }
        
        //! Accessor for the generational model object.
        generational_model_type& generational_model() { return _generational_model; }
        
        //! Returns the current update of this EA.
        unsigned long current_update() { return _generational_model.current_update(); }
        
        //! Returns the event handler.
        event_handler_type& events() { return _events; }
        
        //! Returns the configuration object.
        configuration_type& config() { return _configuration; }
        
        //! Accessor for the population model object.
        population_type& get_population(){ return _population; }
        
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
        stop_condition_type _stop; //!< Checks for an early stopping condition.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configuration; //!< User-defined configuration methods.
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
