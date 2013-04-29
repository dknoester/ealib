/* meta_population.h
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
#ifndef _EA_META_POPULATION_H_
#define _EA_META_POPULATION_H_

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
    namespace generational_models {
        
        /*! Default generational model for a metapopulation EA, where all
         subpopulations are updated in lock-step.
         */
		struct meta_population : public generational_model {
			//! Apply this generational model to the meta_population EA.
			template <typename Population, typename EA>
			void operator()(Population& population, EA& ea) {
				BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
				BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
                for(typename Population::iterator i=population.begin(); i!=population.end(); ++i) {
                    (*i)->update();
                }
            }
        };        
    }
    
    /*! Metapopulation evolutionary algorithm, where individuals in the population
     are themselves evolutionary algorithms.
     */
    template <
    typename EA,
    template <typename> class ConfigurationStrategy,
    typename GenerationalModel=generational_models::meta_population,
    template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=default_rng_type>
    class meta_population {
    public:
        //! Configuration object type.
        typedef ConfigurationStrategy<meta_population> configuration_type;
        //! Type of individual held by this metapopulation.
        typedef EA individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! Type of population container.
        typedef ealib::population<individual_type,individual_ptr_type> population_type;
        //! Type of the population container used by the subpopulations:
        typedef typename individual_type::population_type subpopulation_type;
        //! Type of the individual used by the subpopulations:
        typedef typename individual_type::individual_type subindividual_type;
        //! Generational model.
        typedef GenerationalModel generational_model_type;
        //! Event handler.
        typedef EventHandler<meta_population> event_handler_type;
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
        meta_population() {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<meta_population>));
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<individual_type>));
        }
        
        //! Configure this EA.
        void configure() {
            _configurator.configure(*this);
        }
 
        //! Generates the initial populations.
        void initial_population() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->initial_population();
            }
            _configurator.initial_population(*this);
        }

        //! Initialize this and all embedded EAs.
        void initialize() {
            for(std::size_t i=_population.size(); i<get<META_POPULATION_SIZE>(*this); ++i) {
                individual_ptr_type p(new individual_type());
                p->configure();
                p->md() += md();
                put<RNG_SEED>(rng()(std::numeric_limits<int>::max()), *p);
                p->rng().reset(get<RNG_SEED>(*p));
                _population.push_back(p);
            }
            
            for(iterator i=begin(); i!=end(); ++i) {
                i->initialize();
            }
            _configurator.initialize(*this);
        }

        //! Reset all populations.
        void reset() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->reset();
            }
            _configurator.reset(*this);
        }
        
        //! Clear the population.
        void clear() {
            _population.clear();
        }
        
        //! Begin an epoch.
        void begin_epoch() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->begin_epoch();
            }
            _events.record_statistics(*this);
        }
        
        //! End an epoch.
        void end_epoch() {
            for(iterator i=begin(); i!=end(); ++i) {
                i->events().end_of_epoch(*i); // don't checkpoint!
            }
            
            _events.end_of_epoch(*this); // checkpoint!
        }
        
        //! Advance this EA by one update.
        void update() {
            if(!_population.empty()) {
                _generational_model(_population, *this);
            }
            _events.end_of_update(*this);
            
            // update counter and statistics are handled *between* updates:
            _generational_model.next_update();
            _events.record_statistics(*this);
        }
        
        //! Called to build a new empty, but initialized, subpopulation.
        individual_ptr_type make_individual() {
            individual_ptr_type p(new individual_type());
            p->configure();
            p->md() += md();
            put<RNG_SEED>(rng()(std::numeric_limits<int>::max()), *p);
            p->rng().reset(get<RNG_SEED>(*p));
            p->initialize();
            return p;
        }
        
        //! Accessor for the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Accessor for the generational model object.
        generational_model_type& generational_model() { return _generational_model; }
        
        //! Returns the current update of this EA.
        unsigned long current_update() { return _generational_model.current_update(); }

        //! Returns the event handler.
        event_handler_type& events() { return _events; }
        
        //! Returns the configuration object.
        configuration_type& configuration() { return _configurator; }        

        //! Return the population.
        population_type& population() { return _population; }
        
        //! Return the number of embedded EAs.
        std::size_t size() const { return _population.size(); }
        
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

    protected:
        rng_type _rng; //!< Random number generator.
        meta_data _md; //!< Meta-data for the meta-population.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configurator; //!< Configuration object.
        population_type _population; //!< List of EAs in this meta-population.

    private:
        friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);
            
            std::size_t s = size();
            ar & boost::serialization::make_nvp("meta_population_size", s);
            for(const_iterator i=begin(); i!=end(); ++i) {
                ar & boost::serialization::make_nvp("subpopulation", *i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);

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
