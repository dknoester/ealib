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

#include <ea/ancestors.h>
#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/fitness_function.h>
#include <ea/events.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/ptr_population.h>
#include <ea/selection.h>
#include <ea/stop.h>
#include <ea/recombination.h>
#include <ea/rng.h>

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
    , typename UserDefinedConfiguration=default_configuration
    , typename PopulationGenerator=fill_population
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
        //! User-defined configuration methods type.
        typedef UserDefinedConfiguration configuration_type;
        //! Population generator type.
        typedef PopulationGenerator population_generator_type;
        //! Meta-data type.
        typedef meta_data md_type;
        //! Random number generator type.
        typedef default_rng_type rng_type;
        //! Event handler.
        typedef event_handler<evolutionary_algorithm> event_handler_type;
        //! Population type.
        typedef ptr_population<individual_type,individual_ptr_type> population_type;
        //! Iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        //! Reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;

        //! Default constructor.
        evolutionary_algorithm() : _update(0) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<evolutionary_algorithm>));
            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
            _configuration.after_construction(*this);
        }
        
        //! Copy constructor (note that this is *not* a complete copy).
        evolutionary_algorithm(const evolutionary_algorithm& that) {
            _update = that._update;
            _rng = that._rng;
            _fitness_function = that._fitness_function;
            _md = that._md;
            for(const_iterator i=that.begin(); i!=that.end(); ++i) {
                individual_ptr_type q = copy_individual(*i);
                insert(end(),q);
            }
            _configuration.after_construction(*this);
        }

        /*! Assignment operator (note that this is *not* a complete copy).
         
         \warning Not exception safe.
         */
        evolutionary_algorithm& operator=(const evolutionary_algorithm& that) {
            if(this != &that) {
                _update = that._update;
                _rng = that._rng;
                _fitness_function = that._fitness_function;
                _md = that._md;
                clear();
                for(const_iterator i=that.begin(); i!=that.end(); ++i) {
                    individual_ptr_type q = copy_individual(*i);
                    insert(end(),q);
                }
                _configuration.after_construction(*this);
            }
            return *this;
        }

        //! Initializes this EA.
        void initialize() {
            initialize_fitness_function(_fitness_function, *this);
            _configuration.initialize(*this);
        }

        //! Marks the beginning of a new epoch.
        void begin_epoch() {
            _events.record_statistics(*this);
        }
        
        //! Advances this EA by one update.
        void update() {
            if(!_population.empty()) {
                _generational_model(_population, *this);
            }
            _events.end_of_update(*this);
            ++_update;
            _events.record_statistics(*this);
        }
        
        //! Marks the end of an epoch.
        void end_epoch() {
            _events.end_of_epoch(*this);
        }

        //! Resets this EA's RNG seed.
        void reset(unsigned int s) {
            put<RNG_SEED>(s,*this); // save the seed!
            _rng.reset(s);
        }

        //! Returns a new individual built from the given representation.
        individual_ptr_type make_individual(const representation_type& r=representation_type()) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Returns a copy of an individual.
        individual_ptr_type copy_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
            return p;
        }
        
        //! Returns the current update of this EA.
        unsigned long current_update() { return _update; }
        
        //! Returns the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Returns the fitness function object.
        fitness_function_type& fitness_function() { return _fitness_function; }
        
        //! Returns this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Returns this EA's meta-data (const-qualified).
        const md_type& md() const { return _md; }
        
        //! Returns true if this EA should be stopped.
        bool stop() { return _stop(*this); }

        //! Returns the event handler.
        event_handler_type& events() { return _events; }
        
        //! Returns the configuration object.
        configuration_type& config() { return _configuration; }
        
        //! Returns this EA's population.
        population_type& population() { return _population; }
        
        //! Returns the size of this EA's population.
        std::size_t size() const { return _population.size(); }
        
        //! Returns the n'th individual in the population.
        individual_type& operator[](std::size_t n) { return *_population[n]; }
        
        //! Returns a begin iterator to the population.
        iterator begin() { return iterator(_population.begin()); }
        
        //! Returns an end iterator to the population.
        iterator end() { return iterator(_population.end()); }
        
        //! Returns a begin iterator to the population (const-qualified).
        const_iterator begin() const { return const_iterator(_population.begin()); }
        
        //! Returns an end iterator to the population (const-qualified).
        const_iterator end() const { return const_iterator(_population.end()); }
        
        //! Returns a reverse begin iterator to the population.
        reverse_iterator rbegin() { return reverse_iterator(_population.rbegin()); }
        
        //! Returns a reverse end iterator to the population.
        reverse_iterator rend() { return reverse_iterator(_population.rend()); }
        
        //! Returns a reverse begin iterator to the population (const-qualified).
        const_reverse_iterator rbegin() const { return const_reverse_iterator(_population.rbegin()); }
        
        //! Returns a reverse end iterator to the population (const-qualified).
        const_reverse_iterator rend() const { return const_reverse_iterator(_population.rend()); }
        
        //! Inserts individual x into the population before pos.
        void insert(iterator pos, individual_ptr_type x) { _population.insert(pos.base(), x); }
        
        //! Inserts individuals [f,l) into the population before pos.
        template <typename InputIterator>
        void insert(iterator pos, InputIterator f, InputIterator l) { _population.insert(pos.base(), f, l); }
        
        //! Erases the given individual from the population.
        void erase(iterator i) { _population.erase(i.base()); }
        
        //! Erases the given range from the population.
        void erase(iterator f, iterator l) { _population.erase(f.base(), l.base()); }

        //! Erases all individuals in this EA.
        void clear() { _population.clear(); }

    protected:
        unsigned long _update; //!< Update number for this EA.
        rng_type _rng; //!< Random number generator.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        population_type _population; //!< Population instance.
        fitness_function_type _fitness_function; //!< Fitness function object.
        stop_condition_type _stop; //!< Checks for an early stopping condition.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configuration; //!< User-defined configuration methods.
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("population", _population);
        }
    };
    
} // ealib
    
#endif
