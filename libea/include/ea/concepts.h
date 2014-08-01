/* concepts.h
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
#ifndef _EA_CONCEPTS_H_
#define _EA_CONCEPTS_H_

#include <boost/concept_check.hpp>
#include <utility>

namespace ealib {
	namespace detail {
        //! Concept checking helper to ensure that two parameters are the same type.
		template <typename T> void same_type(const T&, const T&);
	}
    
    /*! Meta-data concept.
     
     <b>Refinement of:</b>
	 
	 <b>Models:</b> ealib::metadata.
     */
    template <typename X>
	struct MetaDataConcept : boost::Assignable<X>, boost::CopyConstructible<X> {
	public:
		BOOST_CONCEPT_USAGE(MetaDataConcept) {
		}
	private:
        X x;
	};
    
    /*! Concept to ensure that the given type supports meta-data.

     <b>Refinement of:</b>
	 
	 <b>Models:</b>
     */
    template <typename X>
	struct SupportsMetaDataConcept {
	public:
        typedef typename X::md_type md_type;
        
		BOOST_CONCEPT_USAGE(SupportsMetaDataConcept) {
            BOOST_CONCEPT_ASSERT((MetaDataConcept<md_type>));
            detail::same_type(x.md(), m);
		}
        
        //! Retrieve this individual's meta data.
        md_type& md();
        
        //! Retrieve this individual's meta data (const-qualified).
        const md_type& md() const;

	private:
        X x;
        md_type m;
	};
    
    
    /*! Representation concept.
     
     <b>Refinement of:</b>
	 
	 <b>Models:</b>
     */
    template <typename X>
	struct RepresentationConcept : boost::CopyConstructible<X> {
	public:
		BOOST_CONCEPT_USAGE(RepresentationConcept) {
		}
	private:
        X x;
	};

    /*! Individual concept.
     
     <b>Refinement of:</b>
	 
	 <b>Models:</b> ealib::individual, ealib::organism.
     */
	template <typename X>
	struct IndividualConcept : boost::CopyConstructible<X> {
	public:
        //! Representation type; the "genome."
//        typedef typename X::representation_type representation_type;
        //! Genome type for this individual.
        typedef typename X::genome_type genome_type;
//        //! Phenotype for this individual.
//        typedef typename X::phenotype_type phenotype_type;
//        //! Encoding of this individual.
//        typedef typename X::encoding_type encoding_type;
        //! Traits for this individual.
        typedef typename X::traits_type traits_type;
        
		BOOST_CONCEPT_USAGE(IndividualConcept) {
//            BOOST_CONCEPT_ASSERT((RepresentationConcept<representation_type>));
            BOOST_CONCEPT_ASSERT((SupportsMetaDataConcept<X>));

//            detail::same_type(representation_type(), x.repr());
//            x.repr() = representation_type();
		}

//		//! Retrieve this individual's representation.
//		representation_type& repr();
//        
//		//! Retrieve this individual's representation (const-qualified).
//		const representation_type& repr() const;
//        
        
//        //! Retrieve this individual's attributes.
//        traits_type& traits();
//        
//        //! Retrieve this individual's attributes (const-qualified).
//        const traits_type& traits() const;
//
	private:
		X x; //!< Default constructible.
	};
    
    
    /*! Population concept.
     
     Populations in EALib are actually containers of pointers to individuals, and
     this concept ensures that this is supported by population types.

     <b>Refinement of:</b>
	 
	 <b>Models:</b> ealib::ptr_population.
     */
	template <typename X>
	struct PopulationConcept : boost::CopyConstructible<X>, boost::RandomAccessContainer<X>, boost::BackInsertionSequence<X> {
	public:
//        typedef typename X::individual_type individual_type; //!< Type of individual pointed to by the population.
//        typedef typename X::individual_ptr_type individual_ptr_type; //!< Type of the individual pointer.
        
		BOOST_CONCEPT_USAGE(PopulationConcept) {
//            detail::same_type(i, *p);
		}
	private:
//        individual_type i;
//        individual_ptr_type p;
	};
    
    
    /*! RNG concept.
     
     <b>Refinement of:</b>
	 
	 <b>Models:</b> ealib::rng.
     */
    template <typename X>
	struct RNGConcept : boost::Assignable<X> {
	public:
		BOOST_CONCEPT_USAGE(RNGConcept) {
		}
	private:
	};

    
    /*! Concept to ensure that the given type supplies a random number generator.
     
     <b>Refinement of:</b>
	 
	 <b>Models:</b>
     */
    template <typename X>
	struct SuppliesRNGConcept {
	public:
        typedef typename X::rng_type rng_type;
        
		BOOST_CONCEPT_USAGE(SuppliesRNGConcept) {
            BOOST_CONCEPT_ASSERT((RNGConcept<rng_type>));
            detail::same_type(x.rng(), rng);
		}
	private:
        X x;
        rng_type rng;
	};

    
    /*! Evolutionary algorithm concept, used to ensure the stability of the evolutionary_algorithm
	 interface.
	 
	 <b>Refinement of:</b>
	 
	 <b>Models:</b> ealib::evolutionary_algorithm, ealib::digital_evolution, 
     ealib::novelty_search, ealib::meta_population.
	 
	 */
	template <typename X>
	struct EvolutionaryAlgorithmConcept : boost::CopyConstructible<X>, SupportsMetaDataConcept<X>, SuppliesRNGConcept<X> {
	public:
        typedef typename X::individual_type individual_type; //!< Individual; an "agent."
        typedef typename X::individual_ptr_type individual_ptr_type; //!< Pointer type to an individual.
        typedef typename X::population_type population_type; //!< Population; a container for individuals.
        typedef typename X::iterator iterator; //!< Iterator over a population.
        typedef typename X::const_iterator const_iterator; //!< Const iterator over a population.
        typedef typename X::reverse_iterator reverse_iterator; //!< Reverse iterator over a population.
        typedef typename X::const_reverse_iterator const_reverse_iterator; //!< Const reverse iterator over a population.
		
		BOOST_CONCEPT_USAGE(EvolutionaryAlgorithmConcept) {
            BOOST_CONCEPT_ASSERT((PopulationConcept<population_type>));
            BOOST_CONCEPT_ASSERT((IndividualConcept<individual_type>));
            x.reset_rng(1);
            x.update();
		}
        
	private:
		X x;
	};
    
    
    
    
    
    
    template <typename X>
	struct DigitalOrganismConcept : boost::CopyConstructible<X> {
	public:
        
    private:
    };
    
    
    template <typename X>
	struct DigitalEvolutionConcept : boost::CopyConstructible<X> {
	public:
        typedef typename X::individual_type individual_type;
        
        BOOST_CONCEPT_USAGE(DigitalEvolutionConcept) {
            BOOST_CONCEPT_ASSERT((DigitalOrganismConcept<individual_type>));
		}
        
    private:
    };
    
    
    
    
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
	 
	 <b>Models:</b> ealib::tournament, ealib::elitism, ealib::fitness_proportionate, ealib::truncation, ealib::random
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
	 steady_state generations, this is fairly straightforward (all individuals in the population are
	 at the same generation, always).  For other models, generation numbers and ordering
     of operations (death-birth, birth-death) are different.
	 
	 <b>Refinement of:</b> 
	 
	 <b>Associated types:</b>
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ealib::steady_state
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

  
    
	
	
	/*! Fitness function concept, used to check that fitness function types passed as a template
	 parameter to an ealib::evolutionary_algorithm are valid.
	 
	 Fitness functions are responsible for translating a representation to a less-than comparable
	 value that can be used as input to a selection strategy.	 
	 
	 <b>Refinement of:</b> 
	 
	 <b>Associated types:</b> See attributes.
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b> ealib::all_ones, ealib::strong_all_ones, ealib::royal_road, ealib::royal_road_with_ditches,
	 ealib::unit_fitness
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
	 template parameters to an ealib::evolutionary_algorithm are valid.
	 
	 <b>Refinement of:</b> SelectionStrategyConcept
	 
	 <b>Associated types:</b>
	 
	 <b>Valid expressions:</b> See member functions.
	 
	 <b>Models:</b>ealib::truncation, ealib::elitism
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
	
	
	
	
	/*!
	 */
	template <typename X, typename EA>
	struct SchedulingConcept {
		BOOST_CONCEPT_USAGE(SchedulingConcept) {
		}

		X x; //!< SchedulingConcept must be default constructible.
		EA ea;		
	};
		
}

#endif
