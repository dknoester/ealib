/* genetic_algorithm.h
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
#ifndef _EA_GENETIC_ALGORITHM_H_
#define _EA_GENETIC_ALGORITHM_H_

/* WARNING:
 
 This code is in progress, and is likely to change dramatically.
 
 (testing out an easier mech for properties & named template parameters)
 
 */



typedef .... ea_type;
typedef properties<ea_type> props;


void configure(ea,props) {
    props.load(pfile);
    
    
}










cmdline_interface<ea_type> cmdline;
// cmdline may set rng seed & override params

void gather_options(EA& ea) {
    tie(ea.POPULATION_SIZE, "ea.population_size");
    tie(ea.REPRESENTATION_SIZE, "ea.representation_size");
    
}

//
void configure(EA& ea) {
    
}


















/*
 
 configuration strategy == traits
 individual attrs == traits
 
 for single pop EAs, one config file
 for meta pop EAs, two: one for the metapop, one for the subpop
 
 
 there is a difference between having a property, and storing those properties
 somewhere.  bottom line: cannot use unified MD store if we need two components
 to have different values for the same MD element.
 
 to do this, it must be object-based.
 
 */

typedef evolutionary_algorithm
< representation<bitstring>
, mutation_operator<per_site<bit_flip> >
, recombination_operator<two_point_crossover>
, generational_model<moran_process>
> ea_type;

template <typename Parameters, typename EA>
class cmdline_interface : public ea_interface {
public:
    void gather_cmdline_options() {
    }
    
protected:
    
};


template <typename EA>
struct ea_traits : public abstract_ea_traits {
    typedef directS encoding_type;
    typedef typename EA::representation_type phenotype;
    typedef phenotype* phenotype_ptr;
    
    virtual void configure(EA& ea) {
    }
    
    virtual void parameterize(EA& ea) {
    }
    
};


struct moran_process {
    property<double> REPLACEMENT_RATE;
    
    void operator()(EA& ea) {
        ea.POPULATION_SIZE
        
        
    }


};


class property_tier {
public:
    
};

template <typename EA>
void configure(ptree& pt, EA& ea) {
    ptie
    
    
    
}










pmap.tie(ea.moran_process)

typedef evolutionary_algorithm
< representation<bitstring>
, mutation_operator<per_site>
, recombination_operator<two_point_crossover>
, generational_model<moran_process>
> ea_type;

ea_type ea; // this is the EA -- ONLY the EA.  still need to set the runtime params


ea_properties eprop; // this is one way to set the EA properties; plenty others are possible
eprop.tie(&ea.POPULATION_SIZE, "ea.population.size")
.tie(&ea.mutation_operator().PER_SITE_P, "ea.mutation_operator.per_site_p");
eprop.load(ptree / config file / etc);






#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/attributes.h>
#include <ea/concepts.h>
#include <ea/configuration.h>
#include <ea/generational_models/steady_state.h>
#include <ea/individual.h>
#include <ea/phenotype.h>
#include <ea/fitness_function.h>
#include <ea/ancestors.h>
#include <ea/meta_data.h>
#include <ea/mutation.h>
#include <ea/selection.h>
#include <ea/stop.h>
#include <ea/population.h>
#include <ea/recombination.h>
#include <ea/events.h>
#include <ea/rng.h>


#include <iostream>
#include <boost/parameter.hpp>

namespace parameter = boost::parameter;


BOOST_PARAMETER_TEMPLATE_KEYWORD(representation)
BOOST_PARAMETER_TEMPLATE_KEYWORD(fitness_function)
BOOST_PARAMETER_TEMPLATE_KEYWORD(mutation_operator)

using boost::mpl::_;
using namespace boost::parameter;

typedef boost::parameter::parameters<
required<tag::representation, is_class<_> >
, optional<tag::fitness_function, is_class<_> >
, optional<tag::mutation_operator, is_class<_> >
> ea_signature;

template <
class A0,
class A1 = parameter::void_,
class A2 = parameter::void_
>
class evolutionary_algorithm {
public:
    
    // Create ArgumentPack
    typedef typename ea_signature::bind<A0,A1,A2>::type args;
    
    // Extract first logical parameter.
    typedef typename parameter::binding<
    args, tag::representation>::type representation;
    
    typedef typename parameter::binding<
    args, tag::fitness_function>::type fitness_function;
    
    typedef typename parameter::binding<
    args, tag::mutation_operator>::type mutation_operator;
    
    void update() {
        cout << "foo" << endl;
    }
};

int main(int argc, char* argv[]) {
    using namespace ealib;
    
    //evolutionary_function(0, _generational_model=7, _fitness_function=12);
    
    evolutionary_algorithm<
    representation<int>,
    mutation_operator<char>,
    fitness_function<double>
    > ea;
    
    ea.update();
    return 0;
}



namespace ealib {
    
	/*! Canonical genetic algorithm.
     
     Genetic algorithms can be thought of as a collection of routines that 
     stochastically update a population of bitstrings.  Since there are a 
     variety of ways to do this, the details of updating the population are 
     factored into a set of function objects (this is the core idea behind 
     EALib).
     
     This class represents the canonical genetic algorithm (GA).  It uses a 
     bitstring representation, sexual recombination (two-point crossover), has 
     no population structure to speak of, and employs a steady state 
     generational model atop fitness proportionate selection.
     
     The only template parameter that the user is required to supply is a 
     fitness function, though the default options may, of course, be overridden.
     
     From the user's end, declaring a GA can be as simple as this:
     
     struct my_fitness_function {
     template <typename Individual, typename Phenotype, typename EA>
       double operator()(Individual& indiv, Phenotype& pheno, EA& ea) {
         return 1.0;
       }
     };
     
     typedef genetic_algorithm<my_fitness_function> ga_type;
     
     ... and that's it (for declaring the GA).
     
     Of course, this does little for actually *using* the GA.  Running the GA
     requires a bit more work, mostly to configure the various properties that
     control things like population size, mutation rates, etc.  For that, see
     interface.h -- Don't worry, it's easy. :-)
     */
    
    
    
    
    
    template
    < typename FitnessFunction
    , typename Representation=representation::bitstring
	, typename MutationOperator=recombination::two_point_crossover
    , typename GenerationalModel=generational_model::steady_state<selection::proportionate< >, selection::tournament< > >,
    > 
    
    
    template <typename> class ConfigurationStrategy,
    typename RecombinationOperator>
    struct qhfc
    : meta_population<
    // embedded ea type:
    evolutionary_algorithm<Representation,
    MutationOperator,
    FitnessFunction,
    ConfigurationStrategy,
    RecombinationOperator,
    generational_models::deterministic_crowding< > >,
    // mp types:
    mutation::operators::no_mutation,
    constant,
    qhfc_configuration,
    recombination::no_recombination,
    generational_models::qhfc,
    dont_stop,
    attr::no_attributes> {
    };
    
    
    template
    <typename FitnessFunction
    > genetic_algorithm
    : public evolutionary_algorithm
    < representation<bitstring>
    >
    
    
	template <
	typename Representation,
	typename MutationOperator,
	typename FitnessFunction,
    template <typename> class ConfigurationStrategy=abstract_configuration,
	typename RecombinationOperator=recombination::two_point_crossover,
	typename GenerationalModel=generational_models::steady_state<selection::proportionate< >, selection::tournament< > >,
    typename StopCondition=dont_stop,
    template <typename> class IndividualAttrs=attr::default_attributes,
    template <typename> class Individual=individual,
	template <typename,typename> class Population=ealib::population,
	template <typename> class EventHandler=event_handler,
	typename MetaData=meta_data,
	typename RandomNumberGenerator=ealib::default_rng_type>
	class evolutionary_algorithm {
    public:
        //! Tag indicating the structure of this population.
        typedef singlePopulationS population_structure_tag;
        //! Meta-data type.
        typedef MetaData md_type;
        //! Random number generator type.
        typedef RandomNumberGenerator rng_type;
        //! Function that checks for a stopping condition.
        typedef StopCondition stop_condition_type;
        //! Representation type.
        typedef Representation representation_type;
        //! Fitness function type.
        typedef FitnessFunction fitness_function_type;
        //! Fitness type.
        typedef typename fitness_function_type::fitness_type fitness_type;
        //! Attributes attached to individuals.
        typedef IndividualAttrs<evolutionary_algorithm> individual_attr_type;
        //! Individual type.
        typedef Individual<evolutionary_algorithm> individual_type;
        //! Individual pointer type.
        typedef boost::shared_ptr<individual_type> individual_ptr_type;
        //! Configuration object type.
        typedef ConfigurationStrategy<evolutionary_algorithm> configuration_type;
        //! Phenotype.
        typedef typename configuration_type::phenotype phenotype;
        //! Encoding type.
        typedef typename configuration_type::encoding_type encoding_type;
        //! Ancestor generator type.
        typedef typename configuration_type::representation_generator_type representation_generator_type;
        //! Mutation operator type.
        typedef MutationOperator mutation_operator_type;
        //! Crossover operator type.
        typedef RecombinationOperator recombination_operator_type;
        //! Generational model type.
        typedef GenerationalModel generational_model_type;
        //! Population type.
        typedef Population<individual_type, individual_ptr_type> population_type;
        //! Event handler.
        typedef EventHandler<evolutionary_algorithm> event_handler_type;
        //! Iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::iterator> iterator;
        //! Const iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_iterator> const_iterator;
        //! Reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::reverse_iterator> reverse_iterator;
        //! Const reverse iterator over this EA's population.
        typedef boost::indirect_iterator<typename population_type::const_reverse_iterator> const_reverse_iterator;
        
        //! Default constructor.
        evolutionary_algorithm() {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<evolutionary_algorithm>));
            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
            configure();
        }
        
        //! Copy constructor (note that this is *not* a complete copy).
        evolutionary_algorithm(const evolutionary_algorithm& that) {
            _rng = that._rng;
            _fitness_function = that._fitness_function;
            _md = that._md;
            // gm doesn't copy...
            // events doesn't copy...
            // configurator doesn't copy...
            // copy individuals:
            for(const_iterator i=that.begin(); i!=that.end(); ++i) {
                individual_ptr_type q = make_individual(*i);
                append(q);
            }
            configure();
        }
        
        //! Configure this EA.
        void configure() {
            _configurator.configure(*this);
        }
        
        //! Build the initial population.
        void initial_population() {
            generate_ancestors(representation_generator_type(), get<POPULATION_SIZE>(*this)-_population.size(), *this);
        }
        
        //! Initialize this EA.
        void initialize() {
            initialize_fitness_function(_fitness_function, *this);
            _configurator.initialize(*this);
        }
        
        //! Reset the population.
        void reset() {
            nullify_fitness(_population.begin(), _population.end(), *this);
            _configurator.reset(*this);
        }
        
        //! Reset the RNG.
        void reset_rng(unsigned int s) {
            put<RNG_SEED>(s,*this); // save the seed!
            _rng.reset(s);
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
            if(!_population.empty()) {
                _generational_model(_population, *this);
            }
            _events.end_of_update(*this);
            
            _generational_model.next_update();
            _events.record_statistics(*this);
        }
        
        //! Returns trus if this EA should be stopped.
        bool stop() {
            return _stop(*this);
        }
        
        //! Build an individual from the given representation.
        individual_ptr_type make_individual(const representation_type& r=representation_type()) {
            individual_ptr_type p(new individual_type(r));
            return p;
        }
        
        //! Build a copy of an individual.
        individual_ptr_type make_individual(const individual_type& ind) {
            individual_ptr_type p(new individual_type(ind));
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
        stop_condition_type _stop; //!< Checks for an early stopping condition.
        generational_model_type _generational_model; //!< Generational model instance.
        event_handler_type _events; //!< Event handler.
        configuration_type _configurator; //!< Configuration object.
        population_type _population; //!< Population instance.
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("rng", _rng);
            ar & boost::serialization::make_nvp("fitness_function", _fitness_function);
            ar & boost::serialization::make_nvp("population", _population);
            ar & boost::serialization::make_nvp("generational_model", _generational_model);
            ar & boost::serialization::make_nvp("meta_data", _md);
        }
    };
    
} // ea

#endif
