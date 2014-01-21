/* novelty_search.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester, Randal S. Olson.
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
#ifndef _EA_NOVELTY_SEARCH_H_
#define _EA_NOVELTY_SEARCH_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/attributes.h>
#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/comparators.h>
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
    
    //! Novelty attribute.
    template <typename EA>
    struct novelty_attribute {
        typedef std::vector<double> novelty_type; //!< This individual's location in phenotype space.
        
        novelty_type& novelty() { return _v; }
        
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("novelty", _v);
        }
        
        novelty_type _v;
    };
    
    //! Novelty accessor method.
    template <typename EA>
    typename EA::individual_type::attr_type::novelty_type& novelty(typename EA::individual_type& ind, EA& ea) {
        return ind.attr().novelty();
    }
    
    //! Default attributes for a novelty_search individual.
    template <typename EA>
    struct default_ns_attributes : attr::fitness_attribute<EA>, novelty_attribute<EA> {
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("fitness_attr", boost::serialization::base_object<attr::fitness_attribute<EA> >(*this));
            ar & boost::serialization::make_nvp("novelty_attr", boost::serialization::base_object<novelty_attribute<EA> >(*this));
        }
    };
    
    /*! Novelty search evolutionary algorithm.
     
     In contrast to traditional evolutionary algorithms, novelty search is
     "objectiveless," in the sense that individuals with higher fitness are not
     necessarily preferentially replicated.  Instead, those individuals that are
     most "novel" reproduce more frequently.
     */
    template <
	typename Representation,
	typename MutationOperator,
	typename FitnessFunction,
    typename NoveltyMetric,
    template <typename> class ConfigurationStrategy=abstract_configuration,
	typename RecombinationOperator=recombination::two_point_crossover,
	typename GenerationalModel=generational_models::steady_state<selection::proportionate< >, selection::tournament< > >,
    template <typename> class IndividualAttrs=default_ns_attributes,
    template <typename> class Individual=individual,
	template <typename,typename> class Population=ealib::population,
	template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ealib::default_rng_type>
	class novelty_search {
    public:
        //! Tag indicating the structure of this population.
        typedef singlePopulationS population_structure_tag;
        //! Configuration object type.
        typedef ConfigurationStrategy<novelty_search> configuration_type;
        //! Representation type.
        typedef Representation representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::fitness_type fitness_type;
        //! Novelty metric.
        typedef NoveltyMetric novelty_metric_type;
        //! Attributes attached to individuals.
        typedef IndividualAttrs<novelty_search> individual_attr_type;
        //! Individual type.
        typedef Individual<novelty_search> individual_type;
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
        typedef EventHandler<novelty_search> event_handler_type;
        //! Iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        //! Reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;
        
        //! Default constructor.
        novelty_search() {
//            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<novelty_search>));
//            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
        }
        
        //! Configure this EA.
        void configure() {
            _configurator.configure(*this);
        }
        
        //! Genesis; create the initial population.
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
            _configurator.reset(*this);
        }

        //! Clear the population.
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
            
            // update counter, fitness, and statistics are handled *between* updates:
            _generational_model.next_update();
            relativize();
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
        
        //! Retrieve the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Retrieve this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Retrieve the fitness function.
        fitness_function_type& fitness_function() { return _fitness_function; }
        
        //! Retrieve the generational model object.
        generational_model_type& generational_model() { return _generational_model; }
        
        //! Retrieve the current update number.
        unsigned long current_update() { return _generational_model.current_update(); }

        //! Retrieve the event handler.
        event_handler_type& events() { return _events; }

        //! Returns the configuration object.
        configuration_type& configuration() { return _configurator; }
        
        //! Retrieve the archive of novel individuals.
        population_type& archive() { return _archive; }
        
        //! Retrieve the list of objectively fittest individuals.
        population_type& fittest() { return _fittest; }

        //! Retrieve the population.
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

        //! Relativize fitness values of individuals in the range [f,l).
        void relativize() {
            int archive_add_count = 0;
            
            for(typename population_type::iterator i=_population.begin(); i!=_population.end(); ++i) {
                std::vector<double> nearest_neighbors(_archive.size() + _population.size());
                
                for(typename population_type::iterator j=_population.begin(); i!=_population.end(); ++j) {
                    if (i != j) {
                        nearest_neighbors.push_back(algorithm::vdist((**i).attr().novelty().begin(),
                                                                     (**i).attr().novelty().end(),
                                                                     (**j).attr().novelty().begin(),
                                                                     (**j).attr().novelty().end()));
                    }
                }
                
                for(typename population_type::iterator j=_archive.begin(); j!=_archive.end(); ++j) {
                    nearest_neighbors.push_back(algorithm::vdist((**i).attr().novelty().begin(),
                                                                 (**i).attr().novelty().end(),
                                                                 (**j).attr().novelty().begin(),
                                                                 (**j).attr().novelty().end()));
                }
                
                // sort novelty distances ascending
                std::sort(nearest_neighbors.begin(), nearest_neighbors.end());
                
                ealib::fitness(**i,*this) = algorithm::vmean(nearest_neighbors.begin(),
                                                      nearest_neighbors.begin() + get<NOVELTY_NEIGHBORHOOD_SIZE>(*this),
                                                      0.0);
                
                // add highly novel individuals to the archive:
                if(ealib::fitness(**i,*this) > get<NOVELTY_THRESHOLD>(*this)) {
                    _archive.insert(_archive.end(),*i);
                    ++archive_add_count;
                }
                
                // update the fittest list -- base this on objective fitness
                _fittest.insert(_fittest.end(),*i);
                if(_fittest.size() > static_cast<std::size_t>(get<NOVELTY_FITTEST_SIZE>(*this))) {
                    std::sort(_fittest.begin(), _fittest.end(), comparators::fitness_desc<novelty_search>(*this));
                    _fittest.resize(get<NOVELTY_FITTEST_SIZE>(*this));
                }
            }
            
            // adjust the archive threshold, if necessary
            if(archive_add_count > 3) {
                get<NOVELTY_THRESHOLD>(*this) *= 1.1;
            } else if(archive_add_count == 0) {
                get<NOVELTY_THRESHOLD>(*this) *= 0.9;
            }
        }

        rng_type _rng; //!< Random number generator.
        fitness_function_type _fitness_function; //!< Fitness function object.
        population_type _population; //!< Population instance.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configurator; //!< Configuration object.
        population_type _archive; //!< Archive of novel individuals.
        population_type _fittest; //!< List of objectively fittest individuals.
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("archive", _archive);
            ar & boost::serialization::make_nvp("fittest", _fittest);
        }
    };
    
} // ea

#endif
