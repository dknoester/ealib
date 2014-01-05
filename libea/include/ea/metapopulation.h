/* metapopulation.h
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
#ifndef _EA_METAPOPULATION_H_
#define _EA_METAPOPULATION_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <limits>

#include <ea/configuration.h>
#include <ea/concepts.h>
#include <ea/ancestors.h>
#include <ea/population.h>
#include <ea/meta_data.h>
#include <ea/events.h>
#include <ea/rng.h>
#include <ea/generational_models/isolated_subpopulations.h>
#include <ea/fitness_functions/constant.h>
#include <ea/mutation.h>
#include <ea/recombination.h>
#include <ea/stop.h>
#include <ea/traits.h>

namespace ealib {
    
    LIBEA_MD_DECL(META_POPULATION_SIZE, "ea.metapopulation.size", unsigned int);
    LIBEA_MD_DECL(METAPOP_COMPETITION_PERIOD, "ea.metapopulation.competition_period", unsigned int);

    template
    < typename EA
    , typename FitnessFunction=constant
    , template <typename> class Traits=default_traits
    > struct subpopulation : public EA {
        //! Subpopulation EA type.
        typedef EA ea_type;
        //! Synonym for the EA type.
        typedef ea_type representation_type;
        //! Fitness value type for this individual.
        typedef typename FitnessFunction::fitness_type fitness_type;
        //! Phenotype for this individual.
        typedef ea_type phenotype_type;
        //! Encoding of this individual.
        typedef directS encoding_type;
        //! Pointer to this individual.
        typedef boost::shared_ptr<subpopulation> individual_ptr_type;
        //! Traits for this individual.
        typedef Traits<subpopulation> traits_type;
        //! Phenotype pointer type.
        typedef typename traits_type::phenotype_ptr_type phenotype_ptr_type;

        //! Default constructor.
        subpopulation() {
        }
        
        //! Copy constructor.
        subpopulation(const ea_type& ea) : EA(ea) {
        }

        //! Retrieve this individual's attributes.
        traits_type& traits() { return _traits; }
        
        //! Retrieve this individual's attributes (const-qualified).
        const traits_type& traits() const { return _traits; }
        
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("ea", boost::serialization::base_object<ea_type>(*this));
            ar & boost::serialization::make_nvp("traits", _traits);
		}

        traits_type _traits; //!< This subpopulation's attributes.
    };

    
    /*! Metapopulation evolutionary algorithm, where individuals in the population
     are themselves evolutionary algorithms.
     
     By default, a meta-population EA provides something akin to an island model,
     where the subpopulations are completely isolated from one another, and individuals
     do not migrate among subpopulations.
     */
    template
    < typename Subpopulation
    , typename AncestorGenerator=ancestors::default_representation
	, typename FitnessFunction=constant
    , typename MutationOperator=mutation::operators::no_mutation
    , typename RecombinationOperator=recombination::no_recombination
    , typename GenerationalModel=generational_models::isolated_subpopulations
    , typename EarlyStopCondition=dont_stop
    , typename Configuration=default_configuration
    , typename MetaData=meta_data
    , typename RandomNumberGenerator=default_rng_type
    > class metapopulation {
    public:
        //! Tag indicating the structure of this population.
        typedef multiPopulationS population_structure_tag;
        //! Subpopulation type (which are themselves EAs).
        typedef Subpopulation subpopulation_type;
        //! Subpopulation pointer type.
        typedef boost::shared_ptr<subpopulation_type> subpopulation_ptr_type;
        //! Synonym for sub_population_type.
        typedef subpopulation_type individual_type;
        //! Synonym for sub_population_ptr_type.
        typedef subpopulation_ptr_type individual_ptr_type;
        //! Synonym for the EA type.
        typedef typename subpopulation_type::representation_type representation_type;
        //! Ancestor generator type.
        typedef AncestorGenerator ancestor_generator_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::value_type fitness_value_type;
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
        typedef event_handler<metapopulation> event_handler_type;
        //! Population type.
        typedef population<subpopulation_type,subpopulation_ptr_type> population_type;
        //! Iterator over the subpopulations in this metapopulation.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        //! Reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;

        //! Construct a meta-population EA.
        metapopulation() {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<metapopulation>));
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<individual_type>));
            configure();
        }
        
        //! Configure this EA.
        void configure() {
            _configurator.configure(*this);
        }
 
        /*! Generate the initial population.
         
         \warning This constructs, initializes, and generates the initial population
         for all of the subpopulations.
         */
        void initial_population() {
            // construct the ancestral subpopulations:
            generate_ancestors(ancestor_generator_type(), get<META_POPULATION_SIZE>(*this)-_population.size(), *this);
            
            // initialize and build the initial populations for each subpopulation:
            for(iterator i=begin(); i!=end(); ++i) {
                i->initialize();
                i->initial_population();
            }
        }

        /*! Initialize the meta-population.
         
         \warning: This does not initialize the subpopulations.
         */
        void initialize() {
            initialize_fitness_function(_fitness_function, *this);
            _configurator.initialize(*this);
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

        //! Returns trus if this EA should be stopped.
        bool stop() {
            return _stop(*this);
        }

        //! Make a new subpopulation from a representation (ea_type).
        individual_ptr_type make_individual(const representation_type& r=representation_type()) {
            individual_ptr_type p(new individual_type(r));
            p->md() += md(); // WARNING: Meta-data comes from the meta-population.
            p->reset_rng(_rng.seed());
            p->initialize();
            return p;
        }
        
        //! Make a new subpopulation from a copy of another.
        individual_ptr_type make_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
            p->md() += ind.md(); // WARNING: Meta-data comes from the individual.
            p->reset_rng(_rng.seed());
            p->initialize();
            return p;
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
        fitness_function_type _fitness_function; //!< Fitness function object.
        meta_data _md; //!< Meta-data for the meta-population.
        stop_condition_type _stop; //!< Checks for an early stopping condition.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configurator; //!< Configuration object.
        population_type _population; //!< List of EAs in this meta-population.

    private:
        friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);
            
            std::size_t s = size();
            ar & boost::serialization::make_nvp("metapopulation_size", s);
            for(const_iterator i=begin(); i!=end(); ++i) {
                ar & boost::serialization::make_nvp("subpopulation", *i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);

            std::size_t s;
            ar & boost::serialization::make_nvp("metapopulation_size", s);
            for(std::size_t i=0; i<s; ++i) {
                individual_ptr_type p(new individual_type());
                p->configure();
                ar & boost::serialization::make_nvp("subpopulation", *p);
                _population.push_back(p);
            }
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
}

#endif
