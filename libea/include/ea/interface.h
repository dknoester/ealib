#ifndef _EA_INTERFACE_H_
#define _EA_INTERFACE_H_

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <algorithm>
#include <iterator>
#include <ea/concepts.h>
#include <ea/meta_data.h>
#include <ea/generators.h>


/*! The templated methods here define the interface to various pieces of an evolutionary
 algorithm.  The idea is that these methods are the public interface, hiding the
 specifics of a given evolutionary algorithm from users.
 */
namespace ea {
    
    /*! Generates an initial population.
     */
    template <typename EA>
    void generate_initial_population(EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        typename EA::initializer_type init;
        init(ea);
    }

	
    /*! Generate n individuals into population p from generator ig.
	 */
	template <typename Population, typename IndividualGenerator, typename EA>
	void generate_individuals_n(Population& p, IndividualGenerator &ig, std::size_t n, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((IndividualGeneratorConcept<IndividualGenerator,EA>));
        std::insert_iterator<Population> ii(p, p.end());
        for( ; n>0; --n) {
            *ii++ = ig(ea);
        }
	}


    /*! Common inheritance details.
     */
    template <typename EA>
    void inherits_from(typename EA::individual_type& parent, typename EA::individual_type& offspring, EA& ea) {
        offspring.name() = next<INDIVIDUAL_COUNT>(ea);
        offspring.generation() = parent.generation() + 1.0;
        offspring.update() = ea.current_update();
    }        
    
    
    /*! Common inheritance details.
     */
    template <typename Population, typename EA>
    void inherits(Population& parents, Population& offspring, EA& ea) {
        for(typename Population::iterator i=offspring.begin(); i!=offspring.end(); ++i) {
            inherits_from(ind(parents.begin(),ea), ind(i,ea), ea);
            ea.events().inheritance(parents, ind(i,ea), ea);
        }
    }
    
    
    /*! Recombine parents to generate offspring via the given recombination operator.
     */
    template <typename Population, typename Recombinator, typename EA>
    void recombine(Population& parents, Population& offspring, Recombinator rec, EA& ea) {
        rec(parents, offspring, ea);
        inherits(parents, offspring, ea);
    }
    
    
    /*! Recombine parents selected from the given population to generate n offspring.
     */
    template <typename Population, typename Selector, typename Recombinator, typename EA>
    void recombine_n(Population& population, Population& offspring, Selector sel, Recombinator rec, std::size_t n, EA& ea) {
        while(offspring.size() < n) {
            Population p, o; // parents, offspring
            sel(population, p, rec.capacity(), ea); // select parents
            rec(p, o, ea); // recombine parents to produce offspring
            inherits(p, o, ea);
            offspring.append(o.begin(), o.end());
        }
        offspring.resize(n); // in case extra were generated...
    }


	/*! Select n individuals from src into dst using the given selector type.
     
     This is "survivor selection" -- The near-final step of most generational models,
     immediately prior to population swaps (if any).  As such, this is where relative 
     fitness is calculated, if the fitness function specifies it.
	 */
	template <typename Selector, typename Population, typename EA>
	void select_n(Population& src, Population& dst, std::size_t n, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
        relativize_fitness(src.begin(), src.end(), ea);
        Selector select(n,src,ea);
        select(src, dst, n, ea);
	}
    
    
	/*! Unconditionally mutate an individual.
	 */	
	template <typename Mutator, typename EA>
	void mutate(typename EA::individual_type& ind, Mutator& mutator, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((MutationOperatorConcept<Mutator,EA>));	
		mutator(ind.repr(), ea);
	}
	
	
	/*! Unconditionally mutate an individual using the EA's embedded types.
	 */
	template <typename EA>
	void mutate(typename EA::individual_type& ind, EA& ea) {
		typename EA::mutation_operator_type mutator;
		mutate(ind, mutator, ea);
	}
    
	
	/*! Unconditionally mutate a range of individuals.
	 */
	template <typename ForwardIterator, typename Mutator, typename EA>
	void mutate(ForwardIterator first, ForwardIterator last, Mutator& mutator, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((MutationOperatorConcept<Mutator,EA>));
		for( ; first!=last; ++first) {
			mutate(ind(first,ea), mutator, ea);
		}		
	}
    
	
	/*! Unconditionally mutate a range of individuals using the EA's embedded types.
	 */
	template <typename ForwardIterator, typename EA>
	void mutate(ForwardIterator first, ForwardIterator last, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		typename EA::mutation_operator_type mutator;
		mutate(first, last, mutator, ea);
	}
	
	
	/*! Probabilistically mutate a range of individuals.
	 */	 
	template <typename ForwardIterator, typename Mutator, typename EA>
	void mutate_p(ForwardIterator first, ForwardIterator last, Mutator& mutator, const double prob, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		BOOST_CONCEPT_ASSERT((MutationOperatorConcept<Mutator,EA>));
		for( ; first!=last; ++first) {
			if(ea.rng().p(prob)) {
				mutate(ind(first,ea), mutator, ea);
			}
		}
	}
	
	
	/*! Probabilisitically mutate a range of individuals using the EA's embedded types.
	 */
	template <typename ForwardIterator, typename EA>
	void mutate_p(ForwardIterator first, ForwardIterator last, const double prob, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		typename EA::mutation_operator_type mutator;
		mutate_p(first, last, mutator, prob, ea);
	}

    //! Retrieve a reference to an individual given a population iterator.
    template <typename EA>
    typename EA::individual_type& ind(typename EA::population_type::iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i);
    }
    
    //! Retrieve a const reference to an individual given a population const_iterator.
    template <typename EA>
    const typename EA::individual_type& ind(typename EA::population_type::const_iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i);
    }
    
    //! Retrieve a reference to an individual given an individual pointer.
    template <typename EA>
    typename EA::individual_type& ind(typename EA::individual_ptr_type p, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return *p;
    }
    
    //! Retrieve a reference to an individual's attributes given a population iterator.
    template <typename EA>
    typename EA::individual_attr_type& attr(typename EA::population_type::iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i).attr();
    }
    
    //! Retrieve a const reference to an individual's attributes given a population const_iterator.
    template <typename EA>
    const typename EA::individual_attr_type& attr(typename EA::population_type::const_iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ind(i).attr();
    }
    
    //! Retrieve a reference to an individual's attributes given an individual pointer.
    template <typename EA>
    typename EA::individual_attr_type& attr(typename EA::individual_ptr_type p, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return p->attr();
    }
    
    template <typename EA>
    const typename EA::individual_ptr_type ptr(typename EA::population_type::const_iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ptr(i);
    }
    
    template <typename EA>
    typename EA::individual_ptr_type ptr(typename EA::population_entry_type i, EA& ea) {
        return ea.population().ptr(i);
    }
    
    template <typename EA>
    typename EA::individual_ptr_type ptr(typename EA::population_type::iterator i, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        return ea.population().ptr(i);
    }
    
    
    

    template <typename EA>
    typename EA::individual_ptr_type make_individual_ptr(const typename EA::individual_type& i, EA& ea) {
        typename EA::individual_ptr_type p(new typename EA::individual_type(i));
        return p;
    }
    
    template <typename EA>
    typename EA::population_entry_type make_population_entry(const typename EA::individual_type& i, EA& ea) {
        typename EA::individual_ptr_type p(new typename EA::individual_type(i));
        return ea.population().make_population_entry(p,ea);
    }
    
    template <typename EA>
    typename EA::population_entry_type make_population_entry(const typename EA::representation_type& r, EA& ea) {
        typename EA::individual_ptr_type p(new typename EA::individual_type(r));
        return ea.population().make_population_entry(p,ea);
    }    
    	    
} // ea

#endif
