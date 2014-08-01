/* evolutionary_algorithm.h
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
#ifndef _EA_EVOLUTIONARY_ALGORITHM_H_
#define _EA_EVOLUTIONARY_ALGORITHM_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/ancestors.h>
#include <ea/checkpoint.h>
#include <ea/concepts.h>
#include <ea/data_structures/shared_ptr_vector.h>
#include <ea/events.h>
#include <ea/individual.h>
#include <ea/lifecycle.h>
#include <ea/metadata.h>
#include <ea/mutation.h>
#include <ea/population_structure.h>
#include <ea/selection.h>
#include <ea/recombination.h>
#include <ea/representation.h>
#include <ea/rng.h>
#include <ea/stopping.h>
#include <ea/traits.h>

namespace ealib {
    
    /*! Generic evolutionary algorithm class.
	 
	 This class is designed to be generic, such that all (most) main features of
     evolutionary algorithms can be incorporated.  The focus of this class is on
     the common features of most EAs, while leaving the problem-specific
     components easily customizable.
	 
     \warning See note below regarding copy construction.
     */
    template
    < typename Representation
    , typename FitnessFunction
	, typename MutationOperator
	, typename RecombinationOperator
	, typename GenerationalModel
    , typename AncestorGenerator
    , typename StopCondition=dont_stop
    , typename PopulationGenerator=fill_population
    , typename Lifecycle=default_lifecycle
    , template <typename> class IndividualTraits=fitness_trait
    > class evolutionary_algorithm {
    public:
        typedef singlePopulationS population_structure_tag;
        typedef Representation representation_type;
        typedef typename representation_type::genome_type genome_type;
        typedef typename representation_type::phenotype_type phenotype_type;
        typedef typename representation_type::encoding_type encoding_type;
        typedef FitnessFunction fitness_function_type;
        typedef typename fitness_function_type::fitness_type fitness_type;
        typedef MutationOperator mutation_operator_type;
        typedef RecombinationOperator recombination_operator_type;
        typedef GenerationalModel generational_model_type;
        typedef AncestorGenerator ancestor_generator_type;
        typedef StopCondition stop_condition_type;
        typedef PopulationGenerator population_generator_type;
        typedef Lifecycle lifecycle_type;
        typedef IndividualTraits<evolutionary_algorithm> individual_traits_type;
        typedef individual<representation_type, individual_traits_type> individual_type;
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        typedef metadata md_type;
        typedef default_rng_type rng_type;
        typedef event_handler<evolutionary_algorithm> event_handler_type;
        typedef shared_ptr_vector<individual_ptr_type> population_type;
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;
        
		//! Internal state of an evolutionary algorithm.
        class state_type {
        public:
            //! Default constructor.
            state_type() : update(0) {
            }
            
            // assignable:
            unsigned long update; //!< Update number for this EA.
            rng_type rng; //!< Random number generator.
            md_type md; //!< Meta-data for this evolutionary algorithm instance.
            fitness_function_type fitness_function; //!< Fitness function object.
            stop_condition_type stop; //!< Checks for an early stopping condition.
            generational_model_type generational_model; //!< Generational model instance.
            lifecycle_type lifecycle; //!< Lifecycle methods.
            
            // these have to be setup via initialization (not assignable):
            event_handler_type events; //!< Event handler.
            
            // these have to be handled carefully:
            population_type population; //!< Population instance.
            
        private:
            state_type(const state_type&);
            state_type& operator=(const state_type&);
            
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version) {
                ar & BOOST_SERIALIZATION_NVP(update);
                ar & BOOST_SERIALIZATION_NVP(rng);
                ar & BOOST_SERIALIZATION_NVP(md);
                ar & BOOST_SERIALIZATION_NVP(population);
            }
        };
        
        //! Default constructor.
        evolutionary_algorithm() {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<evolutionary_algorithm>));
            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
        }
		
        //! Initializing constructor.
        evolutionary_algorithm(const metadata& md) {
            initialize(md);
        }
        
        /*! Copy constructor.
         
         Because the state of an instance of metapopulation is held by the
         state_type, copy construction can be defined in terms of assignment.
         */
        evolutionary_algorithm(const evolutionary_algorithm& that) {
            *this = that;
        }
        
        /*! Assignment operator.
         
         This assignment operator is a bit more powerful than most: First,
         because we hide state behind a separate object, releasing the old state
         is easy.  Second, it's easy to check to see if we in fact need to
         release old state, so it works for copy construction as well.  Finally,
         since state is held in a scoped_ptr, we don't need to worry about
         memory management and exception safety.
         
         \warning This assignment operator is destructive: if something goes
         wrong during assignment, we assume that we're done and that the EA
         should crash.
         */
        evolutionary_algorithm& operator=(const evolutionary_algorithm& that) {
            if(this != &that) {
                if(_state) {
                    _state.reset();
                }
                if(that._state) {
                    // now copy the state of that; let's start by building an
                    // empty EA from that's metadata:
                    initialize(that.md());
                    // at this point, all the non-copyables are configured (e.g.,
                    // events, isa, etc).  copy the easy parts of state first:
                    _state->update = that._state->update;
                    _state->rng = that._state->rng;
                    _state->md = that._state->md;
                    _state->fitness_function = that._state->fitness_function;
                    _state->stop = that._state->stop;
                    _state->lifecycle = that._state->lifecycle;
                    
                    // copy the individuals, link them to the environment, and
                    // update the scheduler:
                    for(const_iterator i=that.begin(); i!=that.end(); ++i) {
                        individual_ptr_type q = copy_individual(*i);
                        insert(end(),q);
                    }
                }
            }
            return *this;
        }
		
        /*! Initialize this EA.
         
         If the state pointer is valid, then we were probably loaded from a
         checkpoint.  In that case, simply update metadata with any potential
         overrides.
         
         If the state pointer is not set, then do one-time initialization tasks.
         
         In both cases, finish by calling lifecycle::after_initialization().
         */
        void initialize(const metadata& md=metadata()) {
            if(!_state) {
                _state.reset(new state_type());
                _state->md = md;
                
                if(exists<RNG_SEED>(*this)) {
                    _state->rng.reset(get<RNG_SEED>(*this));
                } else {
                    unsigned int s = _state->rng.seed();
                    _state->rng.reset(s);
                    put<RNG_SEED>(s, *this);
                }
            } else {
                _state->md += md;
            }
			
            initialize_fitness_function(_state->fitness_function, *this);
            _state->lifecycle.after_initialization(*this);
        }
		
        //! Marks the beginning of a new epoch.
        void begin_epoch() {
            _state->events.record_statistics(*this);
        }
		
        //! Advances this EA by one update.
        void update() {
            if(!_state->population.empty()) {
                _state->generational_model(_state->population, *this);
            }
            _state->events.end_of_update(*this);
            ++_state->update;
            _state->events.record_statistics(*this);
        }
		
        //! Marks the end of an epoch.
        void end_epoch() {
            _state->events.end_of_epoch(*this);
        }
        
		//! Returns a new individual built from the given genome.
        individual_ptr_type make_individual(const genome_type& g=genome_type()) {
            individual_ptr_type p(new individual_type(g));
            return p;
        }
        
        //! Returns a copy of an individual.
        individual_ptr_type copy_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
            return p;
        }

        //! Resets this EA's RNG seed.
        void reset_rng(unsigned int s) {
            put<RNG_SEED>(s,*this); // save the seed!
            _state->rng.reset(s);
        }
		
		//! Returns true if this instance of digital_evolution has state.
        bool has_state() const {
            return _state != 0;
        }
        
        //! Returns the current update of this EA.
        unsigned long current_update() { return _state->update; }
        
        //! Returns the random number generator.
        rng_type& rng() { return _state->rng; }
        
        //! Returns the fitness function object.
        fitness_function_type& fitness_function() { return _state->fitness_function; }
        
        //! Returns this EA's meta-data.
        md_type& md() { return _state->md; }
        
        //! Returns this EA's meta-data (const-qualified).
        const md_type& md() const { return _state->md; }
        
        //! Returns true if this EA should be stopped.
        bool stop() { return _state->stop(*this); }
        
        //! Returns the event handler.
        event_handler_type& events() { return _state->events; }
        
        //! Returns the lifecycle object.
        lifecycle_type& lifecycle() { return _state->lifecycle; }
        
        //! Returns this EA's population.
        population_type& population() { return _state->population; }
        
        //! Returns the size of this EA's population.
        std::size_t size() const { return _state->population.size(); }
        
        //! Returns the n'th individual in the population.
        individual_type& operator[](std::size_t n) { return *_state->population[n]; }
        
        //! Returns a begin iterator to the population.
        iterator begin() { return iterator(_state->population.begin()); }
        
        //! Returns an end iterator to the population.
        iterator end() { return iterator(_state->population.end()); }
        
        //! Returns a begin iterator to the population (const-qualified).
        const_iterator begin() const { return const_iterator(_state->population.begin()); }
        
        //! Returns an end iterator to the population (const-qualified).
        const_iterator end() const { return const_iterator(_state->population.end()); }
        
        //! Returns a reverse begin iterator to the population.
        reverse_iterator rbegin() { return reverse_iterator(_state->population.rbegin()); }
        
        //! Returns a reverse end iterator to the population.
        reverse_iterator rend() { return reverse_iterator(_state->population.rend()); }
        
        //! Returns a reverse begin iterator to the population (const-qualified).
        const_reverse_iterator rbegin() const { return const_reverse_iterator(_state->population.rbegin()); }
        
        //! Returns a reverse end iterator to the population (const-qualified).
        const_reverse_iterator rend() const { return const_reverse_iterator(_state->population.rend()); }
        
        //! Inserts individual x into the population before pos.
        void insert(iterator pos, individual_ptr_type x) { _state->population.insert(pos.base(), x); }
        
        //! Inserts individuals [f,l) into the population before pos.
        template <typename InputIterator>
        void insert(iterator pos, InputIterator f, InputIterator l) { _state->population.insert(pos.base(), f, l); }
        
        //! Erases the given individual from the population.
        void erase(iterator i) { _state->population.erase(i.base()); }
        
        //! Erases the given range from the population.
        void erase(iterator f, iterator l) { _state->population.erase(f.base(), l.base()); }
        
        //! Erases all individuals in this EA.
        void clear() { _state->population.clear(); }
        
    protected:
        boost::scoped_ptr<state_type> _state; //!< Pointer to this EA's letter.
        
    private:
        friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            bool initialized = (_state != 0);
            ar & BOOST_SERIALIZATION_NVP(initialized);
            if(initialized) {
                ar & boost::serialization::make_nvp("state", *_state);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            bool initialized = false;
            ar & BOOST_SERIALIZATION_NVP(initialized);
            if(initialized) {
                _state.reset(new state_type());
                ar & boost::serialization::make_nvp("state", *_state);
            }
        }
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
} // ealib

#endif
