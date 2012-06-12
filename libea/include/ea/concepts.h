/* concepts.h
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
#ifndef _EA_CONCEPTS_H_
#define _EA_CONCEPTS_H_

#include <boost/concept_check.hpp>
#include <utility>

namespace ea {
	namespace detail {
        //! Concept checking helper.
		template <typename T> void same_type(const T&, const T&);
	}
    
    
	/*! Selection strategy concept.
	 
	 A SelectionStrategy encapsulates a strategy used to select individuals from
     a population.  They are typically used for parent and survivor selection, but
     can also be adapted for other purposes, for example, to schedule individuals 
     for execution in the case of an artificial life generational model.
     
     For efficiency, selection strategies operate on populations instead of, for
     example, by providing a generator-compatible interface.
	 
	 <b>Refinement of:</b> 
	 
	 <b>Associated types:</b>
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::tournament, ea::elitism, ea::fitness_proportionate, ea::truncation, ea::random
	 */
	template <typename X, typename Population, typename EA>
	struct SelectionStrategyConcept {
	public:
		BOOST_CONCEPT_USAGE(SelectionStrategyConcept) {
			x(p, p, 1, ea);
		}
		
		/*! Select n individuals from src into dst.
		 
		 <b>Pre-conditions:</b> src must be larger than dst.
		 
		 <b>Post-conditions:</b> dst will contain exactly n individuals; src may be altered.
		 */
		void operator()(Population& src, Population& dst, std::size_t n, EA& ea);

	private:
		X x; Population p; EA ea;
	};	
    
    
	/*! Individual generator concept.
     
     Individual generators are primarily used to support the generation of initial
     populations.  For example, generating an initial population of random individuals.
     
     <b>Refinement of:</b> 
	 
	 <b>Associated types:</b>
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> 
	 */
	template <typename X, typename EA>
	struct IndividualGeneratorConcept {
	public:
		BOOST_CONCEPT_USAGE(IndividualGeneratorConcept) {
            typename EA::population_entry_type i = x(ea);
		}
        
        /*! Generate a single population entry.
		 
		 <b>Pre-conditions:</b>
		 
		 <b>Post-conditions:</b>
		 */
        typename EA::population_entry_type operator()(EA& ea);
        
    private:
        X x; EA ea;
	};
    
    
    /*! Generational model concept.
	 
	 A generational model is meant to encapsulate the transition of one generation to another.  For
	 synchronous generations, this is fairly straightforward (all individuals in the population are
	 at the same generation, always).  For other models, generation numbers and ordering
     of operations (death-birth, birth-death) are different.
	 
	 <b>Refinement of:</b> 
	 
	 <b>Associated types:</b>
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::synchronous_generations
	 */
	template <typename X, typename Population, typename EA>
	struct GenerationalModelConcept {
	public:
		BOOST_CONCEPT_USAGE(GenerationalModelConcept) {
            x(p,ea);
            same_type(1ul, x.current_update());
            x.next_update();
            x.initialize(ea);
		}
        
		//! Retrieve the current generation count.
		unsigned long update() const;
		
		//! Advance the population to the next generation.
        void operator()(Population& population, EA& ea);
        
        //! Initialze this generational model.
        void initialize(EA& ea);

	private:
		X x; Population p; EA ea;
	};

    
    /*! Population concept, used to check that population types passed as a template parameter
	 to an ea::evolutionary_algorithm are valid.
	 
	 A Population is a container that stores and enables access to the individuals stored within it.
	 An individual is actually a tuple that is partially defined by the Population.  Specifically,
	 each individual comprises: {representation, fitness value, population-specific information}.
	 
	 The representation and fitness value components are required, while the population-specific
	 information is optional and defined only by the population.  Population-specific information
	 allows for location information to be attached to an individual, for example.
	 
	 <b>Refinement of:</b> RandomAccessContainer, BackInsertionSequence
	 
	 <b>Associated types:</b> See attributes.
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::unstructured_population
	 
	 \todo Include next-generation functionality.
	 
	 \see http://www.sgi.com/tech/stl/RandomAccessContainer.html
	 */
	template <typename X>
	struct PopulationModelConcept : boost::CopyConstructible<X>, boost::RandomAccessContainer<X>, boost::BackInsertionSequence<X> {
	public:
		BOOST_CONCEPT_USAGE(PopulationModelConcept) {
			// populations are assignable:
			BOOST_CONCEPT_ASSERT((boost::Assignable<X>));
			// and so are the individuals they contain:
			//			BOOST_CONCEPT_ASSERT((boost::Assignable<typename X::individual_type>));
			// fitness comparators must be a model of an adaptable binary predicate:
			//			BOOST_CONCEPT_ASSERT((boost::AdaptableBinaryPredicate<typename X::fitness_comparator_type, 
			//														typename X::individual_type, 
			//														typename X::individual_type>));
			//			same_type(x.less_than(), fc); // must define a factory method for fitness comparators.
			//			x.push_back(r); // must define push_back for representations, w/ default fitness.
			//			x.push_back(r, f); // must define push_back for representations w/ specified fitness.
			//			same_type(i, v); // the individual_type must be the same as the container's value_type.
			//			same_type(x.representation(i), r); // must be able to retrieve a representation from an individual.
			//			same_type(x.fitness(i), f); // must be able to retrieve a fitness value from an individual.
		}
	private:
		//! Appends a new individual with the given representation and fitness to the end of this Population.
		//		void push_back(const typename X::representation_type&, const typename X::fitness_type&);
		
		//! Retrieves a reference to the representation of an individual.
		//		typename X::representation_type& representation(typename X::individual_type&);
		//		
		//		//! Retrieves a reference to the fitness value of an individual.
		//		typename X::fitness_type& fitness(typename X::individual_type&);
		//		
		//		//! Returns a function object that can be used in less-than comparisons.
		//		//typename X::fitness_comparator_type less_than();
		//
		//		X x; //!< Populations must be default constructible.
		//		typename X::representation_type r; //!< Representation type.
		//		typename X::fitness_type f; //!< Fitness value type; a result of fitness evaluation.
		//		typename X::individual_type i; //!< Individual type; stored in Population container.
		//		//typename X::fitness_comparator_type fc; //!< Fitness comparator, used to order individuals by fitness value.
		//		typename X::value_type v; // this is from Container, for convenience only.
		//		template <typename T> void same_type(const T&, const T&); // used to enforce same-typeness.
	};	
    
    
    
    
    
    
    
    
    
    
    	

	template <typename X>
	struct MetaDataConcept {
	public:
		BOOST_CONCEPT_USAGE(MetaDataConcept) {
		}		
	};
	
	
	
		
	
	template <typename X, typename P, typename EA>
	struct GenerableSelectionStrategy {
	public:
		BOOST_CONCEPT_USAGE(GenerableSelectionStrategy) {
			X x(p,ea); // must define a population constructor
			x(p,ea); // must be able to generate an individual without a destination population.
		}	
	private:
		P p;
		EA ea;
	};
	
	

	
	/*! Representations are the genomes of individuals in EALib.  They are the
	 "evolving" part of an evolutionary algorithm.
	 
	 Conceptually, representations in EALib are nothing more than iterable 
	 containers that provide a mutate() method and can be operated upon by 
	 recombination and variation operators.  Iterators here are used as an
	 abstraction so that generic mutation and recombination operators can be
	 written.  Conceptually, each mutable element within a representation can be
	 thought of as a gene.
	 
	 <b>Refinement of:</b>
	 
	 <b>Associated types:</b> See ForwardContainer.
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::network, ea::matrix, ea::bitstring, 
	 ea::realstring.
	 */
	template <typename X, typename EA>
	struct RepresentationConcept : boost::ForwardContainer<X> {
	public:
		BOOST_CONCEPT_ASSERT((boost::Assignable<X>));
		BOOST_CONCEPT_ASSERT((boost::CopyConstructible<X>));

		BOOST_CONCEPT_USAGE(RepresentationConcept) {
			// require a mutate() method that takes an iterator and an ea.
			x.mutate(x.begin(), ea);
		}
		
		/*! Mutate gene i of this representation.
		 */
		void mutate(typename X::iterator i, EA& ea);
		
	private:
		X x;
		EA ea;
	};
	
		
	/*! Matrix representations are matrix-based genomes of individuals in EALib.
	 
	 Matrix representations are refinements of representations.  They enable
	 matrix-specific mutation and recombination operators.
	 
	 <b>Refinement of:</b> Representation.
	 
	 <b>Associated types:</b> See Representation.
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::matrix.
	 */
	template <typename X, typename EA>
	struct MatrixRepresentationConcept: RepresentationConcept<X,EA> {
	public:
		BOOST_CONCEPT_USAGE(MatrixRepresentationConcept) {
		}

	private:
		X x;
		EA ea;
	};
	
	
	/*! Fitness function concept, used to check that fitness function types passed as a template
	 parameter to an ea::evolutionary_algorithm are valid.
	 
	 Fitness functions are responsible for translating a representation to a less-than comparable
	 value that can be used as input to a selection strategy.	 
	 
	 <b>Refinement of:</b> 
	 
	 <b>Associated types:</b> See attributes.
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::all_ones, ea::strong_all_ones, ea::royal_road, ea::royal_road_with_ditches,
	 ea::unit_fitness
	 */
	template <typename X, typename R, typename EA>
	struct FitnessFunctionConcept {
	public:
		typedef typename X::value_type value_type;
		
		BOOST_CONCEPT_ASSERT((boost::LessThanComparable<value_type>));
		
		BOOST_CONCEPT_USAGE(FitnessFunctionConcept) {
			X::null_fitness(); // fitness functions must statically define null_fitness(),
			same_type(X::null_fitness(), f); // which must return a value_type,
			x(r, ea); // and define operator()(R&, ea&),
			same_type(x(r, ea), f); // which must also return a value_type.
			same_type(X::is_null_fitness(f), true); // fitness functions must also define is_null_fitness(), which returns a bool.
		}
		
		/*! Returns an instance of X::value_type signifying that fitness for this
		 representation has not been calculated.
		 */
		static typename X::value_type null_fitness();
		
		//! Test if an X::value_type is equal to the null fitness.
		static bool is_null_fitness(typename X::value_type&);
		
		/*! Calculate the fitness of a representation. */
		typename X::value_type operator()(R& r, EA& ea);
		
		X x; //!< Fitness function objects must be default constructable.
		typename X::value_type f; //!< Fitness value type; the result of fitness evaluations, also default constructible.
		
	private:
		R r;
		EA ea;
		template <typename T> void same_type(const T&, const T&);
	};	
	

	
	/*! Replacement strategy concept, used to check that replacement strategy types passed as
	 template parameters to an ea::evolutionary_algorithm are valid.
	 
	 <b>Refinement of:</b> SelectionStrategyConcept
	 
	 <b>Associated types:</b>
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b>ea::truncation, ea::elitism
	 */
	template <typename X, typename EA>
	struct ReplacementStrategyConcept {
	public:
		BOOST_CONCEPT_USAGE(ReplacementStrategyConcept) {
		}
		
		template <typename ForwardIterator, typename RandomNumberGenerator>
		ForwardIterator replace(ForwardIterator first, ForwardIterator last, RandomNumberGenerator& rng);
		
		template <typename ForwardIterator, typename OutputIterator, typename RandomNumberGenerator>
		void replace_n(ForwardIterator first, ForwardIterator last, OutputIterator result, std::size_t n, RandomNumberGenerator& rng);
		
	private:
		X x;
		EA ea;
		template <typename T> void same_type(const T&, const T&);
	};		
	
	
	/*!
	 */
	template <typename X, typename EA>
	struct MutationOperatorConcept {
	public:
		BOOST_CONCEPT_USAGE(MutationOperatorConcept) {
		}
	private:
		X x;
		EA ea;		
	};
	
	
	/*!
	 */
	template <typename X, typename EA>
	struct RecombinationOperatorConcept {
	public:
		BOOST_CONCEPT_USAGE(RecombinationOperatorConcept) {
		}
	private:
		
		/*! Returns a crossed-over pair of representations.
		 
		 The EA is passed-in so that the crossover operation has access to the random
		 number generator.
		 
		 This representation is crossed-over with that, resulting in two offspring.  The two
		 offspring are formed from (x_0, x_1) cross (y_0, y_1) ==> {(x_0, y_1), (y_0, x_1)}.
		 */		 
		std::pair<X,X> crossover(const X& that, EA& ea) const;
		
		
		X x; //!< RandomNumberGeneratorConcept must be default constructible.
		EA ea;		
	};
	
	
	
	
	

	
	
	
	/*! Meta-population generational model concept, used to check that generational 
	 model types passed as a template parameter to an ea::evolutionary_algorithm are valid.
	 
	 <b>Refinement of:</b> GenerationalModelConcept.
	 
	 <b>Associated types:</b> ea::hierarchical_fair_competition
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::synchronous_generations
	 */
	template <typename X, typename P, typename MD, typename EA>
	struct MetaPopulationModelConcept {
	public:
		BOOST_CONCEPT_USAGE(MetaPopulationModelConcept) {
		}
	private:
		X x;
		EA ea;
	};

	

	
	
	/*!
	 */
	template <typename X, typename EA>
	struct RandomNumberGeneratorConcept {
	public:
		BOOST_CONCEPT_USAGE(RandomNumberGeneratorConcept) {
		}
	private:
		X x; //!< RandomNumberGeneratorConcept must be default constructible.
		EA ea;		
	};
	
	
	/*!
	 */
	template <typename X, typename EA>
	struct SchedulingConcept {
		BOOST_CONCEPT_USAGE(SchedulingConcept) {
		}

		X x; //!< SchedulingConcept must be default constructible.
		EA ea;		
	};
	
	template <typename X>
	struct IndividualConcept {
	public:
		BOOST_CONCEPT_USAGE(IndividualConcept) {
			x.fitness();
		}

	private:
		X x; //!< Default constructible.
	};

	template <typename X>
	struct PopulationConcept {
	public:
		BOOST_CONCEPT_USAGE(PopulationConcept) {
		}
	private:
	};
	
	/*! Evolutionary algorithm concept, used to ensure the stability of the evolutionary_algorithm
	 interface.
	 
	 There should be no need to implement a new evolutionary algorithm type; all customization is
	 preformed through the various types passed as template parameters.
	 
	 <b>Refinement of:</b> 
	 
	 <b>Associated types:</b> See attributes.
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ea::evolutionary_algorithm	 
	 
	 */
	template <typename X>
	struct EvolutionaryAlgorithmConcept {
	public:
//		typedef typename X::ea_type ea_type;
//		typedef typename X::representation_type representation_type;
//		typedef typename X::fitness_function_type fitness_function_type;
//		typedef typename X::fitness_type fitness_type;
//		typedef typename X::individual_type individual_type;
//		typedef typename X::mutation_operator_type mutation_operator_type;
//		typedef typename X::recombination_operator_type recombination_operator_type;
//		typedef typename X::population_type population_type;
//		typedef typename X::parent_selection_type parent_selection_type;
//		typedef typename X::survivor_selection_type survivor_selection_type;
//		typedef typename X::generational_model_type generational_model_type;
//		typedef typename X::rng_type rng_type;
//		typedef typename X::event_handler_type event_handler_type;
		
		BOOST_CONCEPT_USAGE(EvolutionaryAlgorithmConcept) {
		}
	private:
		X x;
	};	
	
}

#endif
