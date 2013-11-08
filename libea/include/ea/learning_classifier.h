/* learning_classifier.h
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
#ifndef _EA_LEARNING_CLASSIFIER_H_
#define _EA_LEARNING_CLASSIFIER_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

#include <ea/attributes.h>
#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/generational_models/moran_process.h>
#include <ea/individual.h>
#include <ea/phenotype.h>
#include <ea/fitness_function.h>
#include <ea/ancestors.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/selection.h>
#include <ea/population.h>
#include <ea/structure.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>
#include <ea/representations/intstring.h>
#include <ea/representations/bitstring.h>
#include <ea/learning_classifier/action.h>
#include <ea/learning_classifier/cover.h>
#include <ea/learning_classifier/environment.h>
#include <ea/learning_classifier/fitness.h>
#include <ea/learning_classifier/match.h>
#include <ea/learning_classifier/message.h>
#include <ea/learning_classifier/reward.h>

namespace ealib {

    //! Probability of running the GA during a given update.
    LIBEA_MD_DECL(LCS_GA_P, "ea.lcs.ga_p", double);
    
    //! Default representation for an LCS.
    struct default_lcs_repr {
        intstring match_string;
        message action_message;
        double bid;
    };
    
    //! Default mutation type for an LCS' representation.
    struct lcs_mutation {

        //! Iterate through all elements in the given representation, possibly mutating them.
        template <typename MutationType, typename Sequence, typename EA>
        void operator()(MutationType mt, Sequence& s, EA& ea) {
            const double per_site_p=get<MUTATION_PER_SITE_P>(ea);
            for(typename Sequence::iterator i=s.begin(); i!=s.end(); ++i){
                if(ea.rng().p(per_site_p)) {
                    mt(i, ea);
                }
            }
        }

        template <typename EA>
        void operator()(typename EA::individual_type& ind, EA& ea) {
            typename EA::representation_type& repr=ind.repr();
            operator()(match, repr.match_string, ea);
            operator()(action, repr.action_message, ea);
            bid(&repr.bid, ea);
        }
        
        mutation::site::bitflip match;
        mutation::site::bitflip action;
        mutation::site::uniform_real bid;
    };
        
    
	/*! Generic learning classifier class.
     
     Learning classifier systems (LCS) are complex adaptive systems that learn and
     adapt to perform the "best" action given its sensed environment.  There are 
     many different variations on LCS, including Michigan- and Pittsburgh-style 
     LCS, as well as others like Hayek~\cite{baum}.  See also ZCS, XCS.

	 The learning_classifier class here is designed to be generic so that it can
     accomodate a wide variety of different LCS approaches.
	 */
	template <
    //typename Environment, // communicates via messages; detectors, effectors
    template <typename> class Environment, // communicates via messages; detectors, effectors
    typename RewardFunction, // acts upon the action set based on feedback from the environment; learning strategy
    typename Representation=default_lcs_repr,
    typename MatchOperator=default_match,
    typename ActionOperator=default_action,
    typename CoveringOperator=no_covering,
    template <typename> class ConfigurationStrategy=abstract_configuration,
    typename FitnessFunction=accuracy_fitness,
	typename MutationOperator=lcs_mutation,
	typename RecombinationOperator=recombination::asexual,
	typename GenerationalModel=generational_models::moran_process<selection::proportionate< >, selection::random>,
    template <typename> class IndividualAttrs=attr::default_attributes,
    template <typename> class Individual=individual,
	template <typename,typename> class Population=ealib::population,
	template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ealib::default_rng_type>
	class learning_classifier {
    public:
        //! Tag indicating the structure of this population.
        typedef singlePopulationS population_structure_tag;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;
        //! Representation type.
        typedef Representation representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::fitness_type fitness_type;
        //! Attributes attached to individuals.
        typedef IndividualAttrs<learning_classifier> individual_attr_type;
        //! Environment type.
        typedef Environment<learning_classifier> environment_type;
        //! Reward function type.
        typedef RewardFunction reward_function_type;
        //! Match operator type.
        typedef MatchOperator match_operator_type;
        //! Action selector type.
        typedef ActionOperator action_operator_type;
        //! Covering operator.
        typedef CoveringOperator covering_operator_type;
        //! Individual type.
        typedef Individual<learning_classifier> individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! Configuration object type.
        typedef ConfigurationStrategy<learning_classifier> configuration_type;
        //! Encoding type.
        typedef typename configuration_type::encoding_type encoding_type;
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
        //! Event handler.
        typedef EventHandler<learning_classifier> event_handler_type;
        //! Iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        //! Reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;
        //! Message board type.
        typedef message_board message_board_type;
        //! Message type.
        typedef message_board_type::message_type message_type;

        //! Default constructor.
        learning_classifier() {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<learning_classifier>));
            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
        }
        
        //! Configure this EA.
        void configure() {
            _configurator.configure(*this);
        }
        
        //! Build the initial population.
        void initial_population() {
            _configurator.initial_population(*this);
        }
        
        //! Initialize this EA.
        void initialize() {
            initialize_fitness_function(_fitness_function, *this);
            _env.initialize(*this);
            _configurator.initialize(*this);
        }
        
        //! Reset the population.
        void reset() {
            nullify_fitness(_population.begin(), _population.end(), *this);
            _configurator.reset(*this);
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
            // i'm pretty sure that these are ephemeral, but this may have to
            // be revisited later:
            population_type actionset, matchset;
            message_board_type env_msgs, action_msgs;

            // get messages from env:
            _env.detectors(env_msgs, *this);
            
            // match current messages against population:
            _match(env_msgs, _population, matchset, *this);
            
            // if we don't have a match, generate a covering rule:
            if(matchset.empty()) {
                _cover(env_msgs, matchset, *this);
            }
            
            // select rules from the match set to be included in the action set,
            // and post their messages:
            _action(matchset, actionset, env_msgs, action_msgs, *this);
            
            // send the current messages back to the environment:
            _env.effectors(action_msgs, *this);
            
            // check for reward:
            _reward(_env, action_msgs, actionset, *this);
            
            // probabilistically evolve the population:
            if(_rng.p(get<LCS_GA_P>(*this)) && !_population.empty()) {
                _generational_model(_population, *this);
            }

            _events.end_of_update(*this);
            _generational_model.next_update();
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
        
        //! Erase the given range from the population.
        void erase(iterator f, iterator l) {
            _population.erase(f.base(), l.base());
        }
        
        //! Accessor for the random number generator.
        rng_type& rng() { return _rng; }
        
        //! Accessor for this EA's meta-data.
        md_type& md() { return _md; }
        
        //! Accessor for the fitness function object.
        fitness_function_type& fitness_function() { return _fitness_function; }
        
        //! Accessor for the environment object.
        environment_type& env() { return _env; }
        
        //! Accessor for the reward function object.
        reward_function_type& reward() { return _reward; }
        
        //! Accessor for the generational model object.
        generational_model_type& generational_model() { return _generational_model; }
        
        //! Returns the current update of this EA.
        unsigned long current_update() { return _generational_model.current_update(); }
        
        //! Returns the event handler.
        event_handler_type& events() { return _events; }
        
        //! Returns the configuration object.
        configuration_type& configuration() { return _configurator; }
        
        //! Accessor for the population model object.
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
        rng_type _rng; //!< Random number generator.
        fitness_function_type _fitness_function; //!< Fitness function object.
        md_type _md; //!< Meta-data for this evolutionary algorithm instance.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configurator; //!< Configuration object.
        population_type _population; //!< Population instance.

        // these are lcs-specific:
        environment_type _env; //!< Environment object.
        reward_function_type _reward; //!< Reward function object; operates on the environment and action set.
//        population_type _match_set; //!< Current matching individuals.
//        population_type _action_set; //!< Current acting individuals.
//        message_board_type _messages; //!< Messages internal to the LCS.
        
        // not serialized:
        match_operator_type _match;
        action_operator_type _action;
        covering_operator_type _cover;
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);
            ar & boost::serialization::make_nvp("meta_data", _md);
            ar & boost::serialization::make_nvp("environment", _env);
//            ar & boost::serialization::make_nvp("reward", _reward);
//            ar & boost::serialization::make_nvp("match_set", _match_set);
//            ar & boost::serialization::make_nvp("action_set", _action_set);
        }
    };
    
} // ea

#endif
