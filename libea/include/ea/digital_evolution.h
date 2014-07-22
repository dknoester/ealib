/* digital_evolution.h
 *
 * This file is part of EALib.
 *
 * Copyright 2014 David B. Knoester, Heather J. Goldsby.
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
#ifndef _EA_DIGITAL_EVOLUTION_H_
#define _EA_DIGITAL_EVOLUTION_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <ea/ancestors.h>
#include <ea/checkpoint.h>
#include <ea/concepts.h>
#include <ea/lifecycle.h>
#include <ea/digital_evolution/events.h>
#include <ea/digital_evolution/ancestors.h>
#include <ea/digital_evolution/environment.h>
#include <ea/digital_evolution/instruction_set.h>
#include <ea/digital_evolution/organism.h>
#include <ea/digital_evolution/schedulers.h>
#include <ea/digital_evolution/replication.h>
#include <ea/digital_evolution/task_library.h>
#include <ea/digital_evolution/resources.h>
#include <ea/metadata.h>
#include <ea/lifecycle.h>
#include <ea/population_structure.h>
#include <ea/data_structures/shared_ptr_vector.h>
#include <ea/recombination.h>
#include <ea/rng.h>
#include <ea/stopping.h>


namespace ealib {
    
    /*! Digital evolution algorithm.
     
     The key difference between digital evolution and standard evolutionary algorithms
     is that individuals here are scheduled for execution, as opposed to having
     their fitness evaluated.  This means that each individual is "visited" more
     than once during each generation, and in fact, the traditional (EA) fitness
     is a function of both the individual's behavior and the population in which
     it lives.  Moreover, replication in an artificial life system is driven
     by the individual, instead of externally (e.g., by a generational model).
     
     A final complicating factor is that individuals in digital evolution interact
     through an "environment."  Such environments are typically responsible for
     handling topology, resource gradients, etc.

     In general, the design of this class is based on concepts from the Avida
     platform for digital evolution~\cite{ofria2004}.
     
     In order to preserve as much compatibility between components as possible,
     the organisms in digital evolution are always referred to as "individuals"
     in code.
     */
    template
    < typename Lifecycle=default_lifecycle
    , typename RecombinationOperator=recombination::asexual
    , typename Scheduler=weighted_round_robin
    , typename AncestorGenerator=selfrep_ancestor
    , typename ReplacementStrategy=random_neighbor
    , typename StopCondition=dont_stop
    , typename PopulationGenerator=generate_single_ancestor
    , template <typename> class IndividualTraits=default_devo_traits
    > class digital_evolution {
    public:
        typedef singlePopulationS population_structure_tag;
        typedef RecombinationOperator recombination_operator_type;
        typedef Scheduler scheduler_type;
        typedef AncestorGenerator ancestor_generator_type;
        typedef ReplacementStrategy replacement_type;
        typedef StopCondition stop_condition_type;
        typedef PopulationGenerator population_generator_type;
        typedef Lifecycle lifecycle_type;
        typedef IndividualTraits<digital_evolution> individual_traits_type;
        typedef organism<individual_traits_type> individual_type;
        typedef typename individual_type::genome_type genome_type;
        typedef typename individual_type::phenotype_type phenotype_type;
        typedef typename individual_type::hardware_type hardware_type;
        typedef typename individual_type::mutation_operator_type mutation_operator_type;
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        typedef metadata md_type;
        typedef default_rng_type rng_type;
        typedef digital_evolution_event_handler<digital_evolution> event_handler_type;
        typedef environment<digital_evolution> environment_type;
        typedef typename environment_type::location_type location_type;
        typedef typename environment_type::location_iterator location_iterator;
        typedef typename environment_type::neighborhood_iterator neighborhood_iterator;
        typedef instruction_set<digital_evolution> isa_type;
        typedef task_library<digital_evolution> task_library_type;
        typedef shared_ptr_vector<individual_ptr_type> population_type;
        typedef resource_vector<digital_evolution> resources_type;
        typedef typename resources_type::resource_ptr_type resource_ptr_type;
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;
        
        /*! Similar to the letter/envelope idiom, here we're defining a type
         that is used to hold the guts of a digital_evolution instance.  The
         problem we're trying to solve here is that we have to provide a way
         to default construct an instance of digital_evolution that doesn't
         actually require initialization, and yet follows the RAII paradigm to
         smooth out the relationship between metapopulations and subpopulations.
         
         Copy/assignment: These are handled by digital_evolution; the state_type
         is simply a container.
         */
        class state_type {
        public:
            //! Default constructor.
            state_type() : update(0) {
            }
            
            // assignable:
            unsigned long update; //!< Update number for this EA.
            rng_type rng; //!< Random number generator.
            md_type md; //!< Meta-data for this evolutionary algorithm instance.
            stop_condition_type stop; //!< Checks for an early stopping condition.
            lifecycle_type lifecycle; //!< Lifecycle methods.
            
            // these have to be setup via initialization (not assignable):
            event_handler_type events; //!< Event handler.
            isa_type isa; //!< Instruction set architecture.
            task_library_type tasklib; //!< Task library.
            resources_type resources; //!< Resources.
            
            // these have to be handled carefully:
            population_type population; //!< Population instance.
            environment_type env; //!< Environment object.
            scheduler_type scheduler; //!< Scheduler instance.

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
                ar & BOOST_SERIALIZATION_NVP(env);
            }
        };
        
        //! Default constructor.
        digital_evolution() {
            BOOST_CONCEPT_ASSERT((DigitalEvolutionConcept<digital_evolution>));
        }
        
        //! Initializing constructor.
        digital_evolution(const metadata& md) {
            initialize(md);
        }
        
        /*! Copy constructor.
         
         Because the state of an instance of digital_evolution is held by the
         state_type, copy construction can be defined in terms of assignment.
         */
        digital_evolution(const digital_evolution& that) {
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
        digital_evolution& operator=(const digital_evolution& that) {
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
                    _state->stop = that._state->stop;
                    _state->lifecycle = that._state->lifecycle;

                    // copy the individuals, link them to the environment, and
                    // update the scheduler:
                    for(const_iterator i=that.begin(); i!=that.end(); ++i) {
                        individual_ptr_type q = copy_individual(*i);
                        insert(end(),q);
                    }
                    _state->env.link(*this);
                    _state->scheduler.link(*this);
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
                
                _state->env.initialize(*this);
            } else {
                _state->md += md;
            }
            
            // for a stateful scheduler, need something like this:
            // _state->scheduler.initialize(*this);
            _state->lifecycle.after_initialization(*this);
        }
        
        //! Marks the beginning of a new epoch.
        void begin_epoch() {
            _state->events.record_statistics(*this);
        }
        
        //! Advances this EA by one update.
        void update() {
            _state->scheduler(_state->population, *this);
            _state->events.end_of_update(*this);
            ++_state->update;
            _state->events.record_statistics(*this);
        }
        
        //! Marks the end of an epoch.
        void end_epoch() {
            _state->events.end_of_epoch(*this);
        }
        
        //! Resets the population (does nothing in digital evolution).
        void reset() {
        }
        
        //! Resets this EA's RNG seed.
        void reset_rng(unsigned int s) {
            put<RNG_SEED>(s,*this); // save the seed!
            _state->rng.reset(s);
        }
        
        //! Builds an individual from the given representation.
        individual_ptr_type make_individual(const genome_type& r=genome_type()) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Builds an individual from the given representation.
        individual_ptr_type copy_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
            return p;
        }
        
        //! Returns true if this instance of digital_evolution has state.
        bool has_state() const {
            return _state != 0;
        }
        
        //! Returns the current update of this EA.
        unsigned long current_update() { return _state->update; }
        
        //! Returns the random number generator.
        rng_type& rng() { return _state->rng; }
        
        //! Returns this EA's meta-data.
        md_type& md() { return _state->md; }
        
        //! Returns this EA's meta-data (const-qualified).
        const md_type& md() const { return _state->md; }
        
        //! Retrieves this AL's environment.
        environment_type& env() { return _state->env; }
        
        //! Returns true if this EA should be stopped.
        bool stop() { return _state->stop(*this); }
        
        //! Returns the event handler.
        event_handler_type& events() { return _state->events; }
        
        //! Returns the lifecycle.
        lifecycle_type& lifecycle() { return _state->lifecycle; }
        
        //! Returns the scheduler.
        scheduler_type& scheduler() { return _state->scheduler; }
        
        //! Retrieves this AL's instruction set architecture.
        isa_type& isa() { return _state->isa; }
        
        //! Retrieves this AL's task library.
        task_library_type& tasklib() { return _state->tasklib; }
        
        //! Returns the resources for this EA.
        resources_type& resources() { return _state->resources; }
        
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
        
        //! Inserts individual x into the population and environment.
        iterator insert(iterator pos, individual_ptr_type x) {
            _state->env.insert(x, *this);
            return iterator(_state->population.insert(pos.base(), x));
        }
        
        //! Inserts individuals [f,l) into the population before pos.
        template <typename InputIterator>
        void insert(iterator pos, InputIterator f, InputIterator l) {
            iterator i=pos;
            for( ; f!=l; ++f, ++i) {
                i=insert(i,*f);
            }
        }
        
        //! Erases the given individual from the population.
        void erase(iterator i) {
            _state->env.erase(*i);
            _state->population.erase(i.base());
        }
        
        //! Erases the given range from the population.
        void erase(iterator f, iterator l) {
            for(iterator i=f; i!=l; ++i) {
                _state->env.erase(*i);
            }
            _state->population.erase(f.base(), l.base());
        }
        
        //! Erases all individuals in this EA.
        void clear() {
            _state->env.clear();
            _state->population.clear();
        }
        
        //! (Re-)Place an offspring in the population, if possible.
        void replace(individual_ptr_type parent, individual_ptr_type offspring) {
            replacement_type r;
            std::pair<location_iterator, bool> l=r(parent, *this);
            
            if(l.second) {
                _state->env.replace(l.first, offspring, *this);
                offspring->priority() = parent->priority();
                _state->population.insert(_state->population.end(), offspring);
                _state->events.birth(*offspring, *parent, *this);
            }
        }
        
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
                _state->env.link(*this);
            }
        }
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
} // ea

#endif
