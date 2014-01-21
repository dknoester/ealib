/* digital_evolution.h
 *
 * This file is part of EALib.
 *
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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

#include <ea/digital_evolution/events.h>
#include <ea/digital_evolution/ancestors.h>
#include <ea/digital_evolution/discrete_spatial_environment.h>
#include <ea/digital_evolution/isa.h>
#include <ea/digital_evolution/organism.h>
#include <ea/digital_evolution/schedulers.h>
#include <ea/digital_evolution/replication.h>
#include <ea/digital_evolution/task_library.h>

#include <ea/ancestors.h>
#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/lifecycle.h>

#include <ea/ptr_population.h>
#include <ea/recombination.h>
#include <ea/rng.h>
#include <ea/stop.h>


namespace ealib {
    
    /*! Artificial life top-level evolutionary algorithm.
     
     The key difference between artificial life and standard evolutionary algorithms
     is that individuals here are scheduled for execution, as opposed to having
     their fitness evaluated.  This means that each individual is "visited" more
     than once during each generation, and in fact, the traditional (EA) fitness
     is a function of both the individual's behavior and the population in which
     it lives.  Moreover, replication in an artificial life system is driven
     by the individual, instead of externally (e.g., by a generational model).
     
     A final complicating factor is that individuals in artificial life interact
     through an "environment."  Such environments are typically responsible for
     handling topology, resource gradients, etc.
     
     While these differences *could* be incorporated into a traditional evolutionary
     algorithm, this could not be done without bastardizing many of the concepts
     that are typically found in an EA.  Thus, the digital_evolution class found
     here.
     
     In general, the design of this class is based on concepts from the Avida
     platform for digital evolution~\cite{ofria2004}.
     
     In order to preserve as much compatibility between EA and AL components,
     the "organisms" in AL are referred to as "individuals."
     */
    template
    < typename Individual
    , typename AncestorGenerator=selfrep_ancestor
	, typename RecombinationOperator=recombination::asexual
    , typename Scheduler=weighted_round_robin
    , typename ReplacementStrategy=random_neighbor
    , typename EarlyStopCondition=dont_stop
    , typename UserDefinedConfiguration=default_configuration
    , typename PopulationGenerator=single_ancestor
    > class digital_evolution {
    public:
        //! Tag indicating the structure of this population.
        typedef singlePopulationS population_structure_tag;
        //! Individual type.
        typedef Individual individual_type;
        //! Individual pointer type.
        typedef typename individual_type::individual_ptr_type individual_ptr_type;
        //! Hardware type.
        typedef typename individual_type::hardware_type hardware_type;
        //! Representation type.
        typedef typename hardware_type::representation_type representation_type;
        //! Mutation operator type.
        typedef typename hardware_type::mutation_operator_type mutation_operator_type;
        //! Ancestor generator type.
        typedef AncestorGenerator ancestor_generator_type;
        //! Recombination operator type.
        typedef RecombinationOperator recombination_operator_type;
        //! Scheduler type.
        typedef Scheduler scheduler_type;
        //! Replacment strategy type.
        typedef ReplacementStrategy replacement_type;
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
        typedef digital_evolution_event_handler<digital_evolution> event_handler_type;
        //! Environment type.
        typedef discrete_spatial_environment<digital_evolution> environment_type;
        //! ISA type.
        typedef isa<digital_evolution> isa_type;
        //! Task library type.
        typedef task_library<digital_evolution> task_library_type;
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
        digital_evolution() : _update(0) {
//            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<digital_evolution>));
//            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
            _configuration.after_construction(*this);
        }
        
        //! Copy constructor (note that this is *not* a complete copy).
        digital_evolution(const digital_evolution& that) {
            _update = that._update;
            _rng = that._rng;
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
        digital_evolution& operator=(const digital_evolution& that) {
            if(this != &that) {
                _update = that._update;
                _rng = that._rng;
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
            _env.initialize(*this);
            _isa.initialize(*this);
            _configuration.initialize(*this);
        }

        //! Marks the beginning of a new epoch.
        void begin_epoch() {
            _events.record_statistics(*this);
        }
        
        //! Advances this EA by one update.
        void update() {
            _scheduler(_population, *this);
            _events.end_of_update(*this);
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

        //! Build an individual from the given representation.
        individual_ptr_type make_individual(const representation_type& r=representation_type()) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Build an individual from the given representation.
        individual_ptr_type copy_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
            return p;
        }

        //! Returns the current update of this EA.
        unsigned long current_update() { return _update; }
        
        //! Returns the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Returns this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Returns this EA's meta-data (const-qualified).
        const md_type& md() const { return _md; }
        
        //! Retrieves this AL's environment.
        environment_type& env() { return _env; }
        
        //! Returns true if this EA should be stopped.
        bool stop() { return _stop(*this); }
        
        //! Returns the event handler.
        event_handler_type& events() { return _events; }
        
        //! Returns the configuration object.
        configuration_type& config() { return _configuration; }
        
        //! Retrieves this AL's instruction set architecture.
        isa_type& isa() { return _isa; }
        
        //! Retrieves this AL's task library.
        task_library_type& tasklib() { return _tasklib; }
        
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

        //! Append individual x to the population and environment.
        void append(individual_ptr_type p) {
            _population.insert(_population.end(), p);
            _env.append(p);
        }
        
        //! Append the range of individuals [f,l) to the population and environment.
        template <typename ForwardIterator>
        void append(ForwardIterator f, ForwardIterator l) {
            _population.insert(_population.end(), f, l);
            _env.append(f, l);
        }
        
        //! (Re-)Place an offspring in the population, if possible.
        void replace(individual_ptr_type parent, individual_ptr_type offspring) {
            replacement_type r;
            std::pair<typename environment_type::iterator, bool> l=r(parent, *this);
            
            if(l.second) {
                _env.replace(l.first, offspring, *this);
                offspring->priority() = parent->priority();
                _population.insert(_population.end(), offspring);
                _events.birth(*offspring, *parent, *this);
            }
        }

    protected:
        unsigned long _update; //!< Update number for this EA.
        rng_type _rng; //!< Random number generator.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        population_type _population; //!< Population instance.
        environment_type _env; //!< Environment object.
        stop_condition_type _stop; //!< Checks for an early stopping condition.
        event_handler_type _events; //!< Event handler.
        configuration_type _configuration; //!< User-defined configuration methods.
        scheduler_type _scheduler; //!< Scheduler instance.
        isa_type _isa; //!< Instruction set architecture.
        task_library_type _tasklib; //!< Task library.
        
    private:
        friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("environment", _env);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("environment", _env);
            
            // now we have to fix up the connection between the environment and organisms:
            _env.attach(*this);
        }
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
} // ea

#endif
