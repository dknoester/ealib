/* metapopulation.h
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
#ifndef _EA_METAPOPULATION_H_
#define _EA_METAPOPULATION_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/lifecycle.h>
#include <ea/concepts.h>
#include <ea/ancestors.h>
#include <ea/data_structures/shared_ptr_vector.h>
#include <ea/population_structure.h>
#include <ea/metadata.h>
#include <ea/events.h>
#include <ea/rng.h>
#include <ea/generational_models/isolated_subpopulations.h>
#include <ea/mutation.h>
#include <ea/recombination.h>
#include <ea/stopping.h> 
#include <ea/traits.h>
#include <ea/subpopulation.h>
#include <ea/fitness_functions/quiet_nan.h>

namespace ealib {
  
    /*! Metapopulation evolutionary algorithm, where individuals in the population
     are themselves evolutionary algorithms.
     
     Conceptually, a metapopulation is a container for evolutionary algorithms.
     
     By default, a meta-population EA provides something akin to an island model,
     where the subpopulations are completely isolated from one another, and individuals
     do not migrate among subpopulations.
     
     Note that a metapopulation EA conforms to the same concepts as a single
     population EA, which means that an "individual" in a metapopulation is an
     entire subpopulation.
     */
    template
    < typename EA
    , typename FitnessFunction=quiet_nan
    , typename MutationOperator=mutation::operators::no_mutation
    , typename RecombinationOperator=recombination::no_recombination
    , typename GenerationalModel=generational_models::isolated_subpopulations
    , typename AncestorGenerator=ancestors::default_subpopulation
    , typename StopCondition=dont_stop
    , typename PopulationGenerator=fill_metapopulation
    , typename Lifecycle=default_lifecycle
    , template <typename> class SubpopulationTraits=fitness_trait
    > class metapopulation {
    public:
        typedef multiPopulationS population_structure_tag;
        typedef EA ea_type;
        typedef FitnessFunction fitness_function_type;
        typedef typename fitness_function_type::fitness_type fitness_type;
        typedef MutationOperator mutation_operator_type;
        typedef RecombinationOperator recombination_operator_type;
        typedef GenerationalModel generational_model_type;
        typedef AncestorGenerator ancestor_generator_type;
        typedef StopCondition stop_condition_type;
        typedef PopulationGenerator population_generator_type;
        typedef Lifecycle lifecycle_type;
        typedef SubpopulationTraits<metapopulation> subpopulation_traits_type;
        typedef subpopulation<ea_type, subpopulation_traits_type> individual_type;
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        typedef individual_type subpopulation_type;
        typedef individual_ptr_type subpopulation_ptr_type;
        typedef metadata md_type;
        typedef default_rng_type rng_type;
        typedef event_handler<metapopulation> event_handler_type;
        typedef shared_ptr_vector<individual_ptr_type> population_type;
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;
        
        //! Construct a meta-population EA.
        metapopulation() : _update(0) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<metapopulation>));
            _lifecycle.after_construction(*this);
        }
        
        //! Copy constructor (note that this is *not* a complete copy).
        metapopulation(const metapopulation& that) {
            _update = that._update;
            _rng = that._rng;
            _fitness_function = that._fitness_function;
            _md = that._md;
            for(const_iterator i=that.begin(); i!=that.end(); ++i) {
                individual_ptr_type q = copy_individual(*i);
                insert(end(),q);
            }
            _lifecycle.after_construction(*this);
        }
        
        /*! Assignment operator (note that this is *not* a complete copy).
         
         \warning Not exception safe.
         */
        metapopulation& operator=(const metapopulation& that) {
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
                _lifecycle.after_construction(*this);
            }
            return *this;
        }
        
        //! Initializes this EA and any existing subpopulations.
        void initialize() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->initialize();
            }
            initialize_fitness_function(_fitness_function, *this);
            _lifecycle.initialize(*this);
        }
        
        //! Begin an epoch.
        void begin_epoch() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->begin_epoch();
            }
            _events.record_statistics(*this);
        }
        
        //! Advance this EA by one update.
        void update() {
            if(!_population.empty()) {
                _generational_model(_population, *this);
            }
            _events.end_of_update(*this);
            ++_update;
            _events.record_statistics(*this);
        }
        
        //! End an epoch.
        void end_epoch() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->events().end_of_epoch(*i); // don't checkpoint!
            }
            _events.end_of_epoch(*this); // checkpoint!
        }
        
        //! Resets the population (does nothing in digital evolution).
        void reset() {
            nullify_fitness(begin(), end(), *this);
        }
        
        //! Resets this EA's RNG seed.
        void reset_rng(unsigned int s) {
            put<RNG_SEED>(s,*this); // save the seed!
            _rng.reset(s);
        }
        
        //! Returns a new individual built from the given individual (subpopulation).
        individual_ptr_type make_individual(const individual_type& r=individual_type()) {
            individual_ptr_type p(new individual_type(r));
            p->md() += md(); // WARNING: Meta-data comes from the meta-population.
            p->reset_rng(_rng.seed());
            p->initialize();
            return p;
        }
        
        //! Returns a copy of an individual.
        individual_ptr_type copy_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
            p->md() += ind.md(); // WARNING: Meta-data comes from the individual.
            p->reset_rng(_rng.seed());
            p->initialize();
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
        
        //! Returns the lifecycle object.
        lifecycle_type& lifecycle() { return _lifecycle; }
        
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
        lifecycle_type _lifecycle; //!< User-defined configuration methods.
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("metadata", _md);
            ar & boost::serialization::make_nvp("population", _population);
        }
    };
    
}

#endif
