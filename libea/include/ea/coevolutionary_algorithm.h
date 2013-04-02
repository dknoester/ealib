/* coevolutionary_algorithm.h
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
#ifndef _EA_COEVOLUTIONARY_ALGORITHM_H_
#define _EA_COEVOLUTIONARY_ALGORITHM_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <limits>

#include <ea/concepts.h>
#include <ea/ancestors.h>
#include <ea/population.h>
#include <ea/meta_data.h>
#include <ea/events.h>
#include <ea/rng.h>

namespace ealib {

    /*! Coevolutionary algorithm.
     
     Conceptually, a coevolutionary algorithm can be thought of as a meta-population
     EA with a fitness function that operates on groups of individuals, instead of
     on a single individual in isolation.
     */
    template <
    typename EA,
    typename GroupSelector,
    typename FitnessFunction,
    template <typename> class ConfigurationStrategy,
    template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ealib::default_rng_type>
    class coevolutionary_algorithm {
    public:
        //! Configuration object type.
        typedef ConfigurationStrategy<meta_population> configuration_type;
        //! Type of individual held by this metapopulation.
        typedef EA individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! Type of population container.
        typedef ealib::population<individual_type,individual_ptr_type> population_type;
        //! Event handler.
        typedef EventHandler<meta_population> event_handler_type;
        //! Selector of groups for fitness evaluation.
        typedef GroupSelector group_selector_type;
        //! Fitness function.
        typename FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::fitness_type fitness_type;
        //! Meta-data type.
        typedef MetaData md_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;
        //! Iterator for embedded EAs.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator for embedded EAs.
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        //! Reverse iterator for embedded EAs.
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator for embedded EAs.
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;
        
        //! Construct a meta-population EA.
        coevolutionary_algorithm() {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<coevolutionary_algorithm>));
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<individual_type>));
            _configurator.construct(*this);
        }
        
        //! Accessor for the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Returns the event handler.
        event_handler_type& events() { return _events; }
        
        //! Return the number of embedded EAs.
        std::size_t size() const {
            return _population.size();
        }
        
        //! Return the population.
        population_type& population() {
            return _population;
        }
        
        //! Return the n'th embedded EAs.
        individual_type& operator[](std::size_t n) {
            return *_population[n];
        }
        
        //! Returns a begin iterator to the embedded EAs.
        iterator begin() {
            return iterator(_population.begin());
        }
        
        //! Returns an end iterator to the embedded EAs.
        iterator end() {
            return iterator(_population.end());
        }
        
        //! Returns a begin iterator to the embedded EAs (const-qualified).
        const_iterator begin() const {
            return const_iterator(_population.begin());
        }
        
        //! Returns an end iterator to the embedded EAs (const-qualified).
        const_iterator end() const {
            return const_iterator(_population.end());
        }
        
        //! Returns a reverse begin iterator to the embedded EAs.
        reverse_iterator rbegin() {
            return reverse_iterator(_population.rbegin());
        }
        
        //! Returns a reverse end iterator to the embedded EAs.
        reverse_iterator rend() {
            return reverse_iterator(_population.rend());
        }
        
        //! Returns a reverse begin iterator to the embedded EAs (const-qualified).
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(_population.rbegin());
        }
        
        //! Returns a reverse end iterator to the embedded EAs (const-qualified).
        const_reverse_iterator rend() const {
            return const_reverse_iterator(_population.rend());
        }
        
        //! Called to build a new (empty) subpopulation.
        individual_ptr_type make_individual() {
            individual_ptr_type p(new individual_type());
            p->md() = md();
            put<RNG_SEED>(rng()(std::numeric_limits<int>::max()), *p);
            p->rng().reset(get<RNG_SEED>(*p));
            p->initialize();
            return p;
        }
        
        //! Initialize this and all embedded EAs, if we have any.
        void initialize() {
            // ok, if we have embedded eas, they've been loaded via a checkpoint.
            // they need to be initialized, but we don't need to create any more.
            // if we don't have any, create & initialize them.
            if(_population.empty()) {
                for(unsigned int i=0; i<get<META_POPULATION_SIZE>(*this); ++i) {
                    _population.push_back(make_individual()); // this also initializes!
                }
            } else {
                for(iterator i=begin(); i!=end(); ++i) {
                    i->initialize();
                }
            }
            _configurator.initialize(*this);
        }
        
        /*! Generates the initial population.  This does nothing at the
         meta-population level, but it does generate the initial populations at
         the sub-population level.
         */
        void generate_initial_population() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->generate_initial_population();
            }
            _configurator.initial_population(*this);
        }
        
        //! Reset all populations.
        void reset() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->reset();
            }
            _configurator.reset(*this);
        }
        
        //! Advance the epoch of this EA by n updates.
        void advance_epoch(std::size_t n) {
            // initial fitness & statistics recording:
            for(iterator i=begin(); i!=end(); ++i) {
                calculate_fitness(i->population().begin(), i->population().end(), *i);
                i->events().record_statistics(*i);
            }
            _events.record_statistics(*this);
            
            // update all the EAs:
            for( ; n>0; --n) {
                update();
            }
            
            // signal end-of-epoch:
            for(iterator i=begin(); i!=end(); ++i) {
                i->events().end_of_epoch(*i); // don't checkpoint!
            }
            
            _events.end_of_epoch(*this); // checkpoint!
        }
        
        /*! Advance this EA by one update.

         Conceptually, what happens here is that a group of individuals is
         selected, and the group is evaluated together.
         */
        void update() {
            group_selector_type gs(*this); // construct the group selector (it can be stateful)
            population_type g=gs(*this); // construct the first group (which is itself a population)
            
            while(!g.empty()) { // while the group isn't empty...
                // calculate fitness of the group, bypassing the normal
                // fitness function control path:
                _fitness_function(g,*this);
                g = gs(*this); // and get the next group to evaluate
            }
            
            // we assume that the _fitness_function has directly set the fitness
            // of individuals in the groups; now, update each subpopulation:
            for(iterator i=begin(); i!=end(); ++i) {
                i->update(); // WARNING: if there's a fitness function defined in
                // the subpopulations, this can do weird things...
            }
            
            _events.end_of_update(*this);
            
            // update counter and statistics are handled between updates:
            ++_update;
            _events.record_statistics(*this);
        }
        
        //! Returns the current update of this EA.
        unsigned long current_update() {
            return _update;
        }
        
    protected:
        unsigned long _update; //!< Coevolutionary algorithm update.
        rng_type _rng; //!< Random number generator.
        fitness_function_type _fitness_function; //!< Fitness function object.
        meta_data _md; //!< Meta-data for the meta-population.
        event_handler_type _events; //!< Event handler.
        population_type _population; //!< List of EAs in this meta-population.
        configuration_type _configurator; //!< Configuration object.
        
    private:
        friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("group_selector", _group_selector);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("meta_data", _md);
            
            std::size_t s = size();
            ar & boost::serialization::make_nvp("meta_population_size", s);
            for(const_iterator i=begin(); i!=end(); ++i) {
                ar & boost::serialization::make_nvp("subpopulation", *i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("update", _update);
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("group_selector", _group_selector);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("meta_data", _md);
            
            std::size_t s;
            ar & boost::serialization::make_nvp("meta_population_size", s);
            for(std::size_t i=0; i<s; ++i) {
                individual_ptr_type p(new individual_type());
                ar & boost::serialization::make_nvp("subpopulation", *p);
                _population.push_back(p);
            }
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
}

#endif
