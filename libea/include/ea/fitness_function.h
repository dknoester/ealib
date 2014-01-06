/* fitness_function.h
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
#ifndef _EA_FITNESS_FUNCTION_H_
#define _EA_FITNESS_FUNCTION_H_

#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <limits>
#include <string>
#include <vector>
#include <ea/algorithm.h>
#include <ea/attributes.h>
#include <ea/meta_data.h>
#include <ea/events.h>

namespace ea {

    /* The following are tags that are to be used to indicate properties of a
     given fitness function:
     */
    
    //! Indicates that fitness is constant, and can be cached.
    struct constantS { };
    
    //! Indicates that fitness may change between evaluations, and should not be cached.
    struct nonstationaryS { };
    
    //! Indicates that fitness of an individual is absolute.
    struct absoluteS { };

    //! Indicates that fitness of an individual is relative to the population.
    struct relativeS { };
    
    //! Indicates that fitness is deterministic, and does not require its own RNG.
    struct deterministicS { };

    //! Indicates that fitness is stochastic, and requires its own RNG.
    struct stochasticS { };
    
    
    /* The following are tags that are used to indicate how the representation for
     an individual is used with respect to fitness evaluation:
     */
    
    //! Indicates a direct encoding, genome used directly in fitness calculation.
    struct directS { };
    
    //! Indicates an indirect encoding, genome is transformed prior to fitness calculation.
    struct indirectS { };
    
    
    /* While fitnesses are typically thought of as a single real value (e.g., a
     double), treating them as an object has numerous benefits, especially for
     multiobjective problems.  Here is the unary fitness value:
     */
    
    /*! Unary fitness value.
     */
    template <typename T>
    struct unary_fitness {
        typedef unary_fitness<T> this_type;
        typedef T value_type;

        //! Constructor.
        unary_fitness() : _f(null()) {
        }
        
        //! Initializing constructor.
        unary_fitness(const value_type& val) : _f(val) {
        }

        //! Destructor.
        virtual ~unary_fitness() {
        }
        
        //! Value-type assignment operator.
        unary_fitness& operator=(const value_type& val) {
            _f = val;
            return *this;
        }

        //! Addition assignment operator.
        unary_fitness& operator+=(const unary_fitness& that) {
            _f += that._f;
            return *this;
        }

        //! Operator <
        bool operator<(const unary_fitness& that) {
            return _f < that._f;
        }

        //! Value-type cast operator.
        operator value_type() { return _f; }

        //! String cast operator.
        operator std::string() { return boost::lexical_cast<std::string>(_f); }

        //! Return the null fitness value.
        value_type null() const { return std::numeric_limits<value_type>::quiet_NaN(); }
        
        //! Nullify this fitness.
        void nullify() { _f = null(); }

        //! Returns true if this fitness is null, false otherwise.
        bool is_null() const { return std::isnan(_f); }

        //! Return the minimum fitness value.
        T minimum() const { return -std::numeric_limits<value_type>::max(); }
        
        //! Returns true if this fitness is minimal, false otherwise.
        bool is_minimum() const { return (_f==minimum()); }
        
        //! Return the maximum fitness value.
        T maximum() const { return std::numeric_limits<value_type>::max(); }
        
        //! Returns true if this fitness is maximal, false otherwise.
        bool is_maximum() const { return (_f==maximum()); }
        
        template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
//			bool null_fitness=is_null();
//			ar & BOOST_SERIALIZATION_NVP(null_fitness);
//			if(!null_fitness) {
				ar & boost::serialization::make_nvp("f", _f);
//			}
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
//			bool null_fitness=true;
//			ar & BOOST_SERIALIZATION_NVP(null_fitness);
//			if(null_fitness) {
//                nullify();
//			} else {
				ar & boost::serialization::make_nvp("f", _f);
//			}
		}
        
        value_type _f; //!< Fitness value.
        
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
    
    /*! Multivalued fitness.
     */
    template <typename T>
    struct multivalued_fitness {
        typedef multivalued_fitness<T> this_type;
        typedef T objective_type;
        typedef std::vector<T> value_type;
        
        //! Constructor.
        multivalued_fitness() : _f(null()) {
        }
        
        //! Initializing constructor.
        multivalued_fitness(const value_type& val) : _f(val) {
        }
        
        //! Destructor.
        virtual ~multivalued_fitness() {
        }
        
        //! Value-type assignment operator.
        multivalued_fitness& operator=(const value_type& val) {
            _f = val;
            return *this;
        }

        //! Value-type cast operator.
        operator value_type() { return _f; }
        
        //! Retrieve the value of objective i.
        objective_type operator[](std::size_t i) { return _f[i]; }
        
        //! Retrieve the value of objective i (const-qualified).
        const objective_type operator[](std::size_t i) const { return _f[i]; }

        //! String cast operator.
        operator std::string() { return algorithm::vcat(_f.begin(), _f.end()); }

        //! Returns the number of fitness values contained.
        std::size_t size() const { return _f.size(); }
        
        //! Return the null fitness value.
        value_type null() const { return value_type(); }
        
        //! Nullify this fitness.
        void nullify() { _f.clear(); }
        
        //! Returns true if this fitness is null, false otherwise.
        bool is_null() const { return _f.empty(); }
        
        //! Serialize this fitness value.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) { 
            ar & boost::serialization::make_nvp("f", _f);
        }
        
        value_type _f; //!< Fitness value.
    };

    namespace detail {

        //! Deterministic initialization (no RNG needed).
        template <typename FitnessFunction, typename EA>
        void initialize_fitness_function(FitnessFunction& ff, deterministicS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            ff.initialize(ea);
        }

        //! Stochastic initialization (RNG needed).
        template <typename FitnessFunction, typename EA>
        void initialize_fitness_function(FitnessFunction& ff, stochasticS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            next<FF_INITIAL_RNG_SEED>(ea);
            typename EA::rng_type rng(get<FF_INITIAL_RNG_SEED>(ea)+1); // +1 to avoid clock
            ff.initialize(rng, ea);
        }
        
    } // detail
    
    template <typename FitnessFunction, typename EA>
    void initialize_fitness_function(FitnessFunction& ff, EA& ea) {
        detail::initialize_fitness_function(ff, typename FitnessFunction::stability_tag(), ea);
    }

    /*! Convenience struct to define typedefs and empty initialization and serialization
     methods.
     */
    template <typename T, 
    typename ConstantTag=constantS,
    typename RelativeTag=absoluteS,
    typename StabilityTag=deterministicS>
    struct fitness_function {
        typedef T fitness_type;
        typedef typename fitness_type::value_type value_type;
        typedef ConstantTag constant_tag;
        typedef RelativeTag relative_tag;
        typedef StabilityTag stability_tag;
        
        //! Initialize this (deterministic) fitness function.
        template <typename EA>
        void initialize(EA& ea) {
        }
        
        //! Initialize this (stochastic) fitness function.
        template <typename RNG, typename EA>
        void initialize(RNG& rng, EA& ea) {
        }
        
        //! Serialize this fitness function.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) { 
        }
    };
    
    
    namespace detail {
        
        //! Calculate fitness for a non-stochastic direct encoding:
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, directS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            fitness(i) = ea.fitness_function()(i, ea);
            ea.events().fitness_evaluated(i,ea);
        }
        
        //! Calculate fitness for a non-stochastic indirect encoding:
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, indirectS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            typename EA::configuration_type::transform_type ti=ea.configuration().transform(i, ea);
            fitness(i) = ea.fitness_function()(i, ti, ea);
            ea.events().fitness_evaluated(i,ea);
        }
        
        //! Calculate fitness for a stochastic direct encoding:
        template <typename EA, typename RNG>
        void calculate_fitness(typename EA::individual_type& i, directS, RNG& rng, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            fitness(i) = ea.fitness_function()(i, rng, ea);
            ea.events().fitness_evaluated(i,ea);
        }

        //! Calculate fitness for a stochastic indirect encoding:
        template <typename EA, typename RNG>
        void calculate_fitness(typename EA::individual_type& i, indirectS, RNG& rng, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            typename EA::configuration_type::transform_type ti=ea.configuration().transform(i, rng, ea);
            fitness(i) = ea.fitness_function()(i, ti, rng, ea);
            ea.events().fitness_evaluated(i,ea);
        }

        //! Deterministic: evaluate fitness without an embedded RNG.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, deterministicS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            calculate_fitness(i, typename EA::encoding_tag(), ea);
        }
        
        //! Stochastic: provide an RNG for use during fitness evaluation.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, stochasticS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            next<FF_RNG_SEED>(ea);
            typename EA::rng_type rng(get<FF_RNG_SEED>(ea)+1); // +1 to avoid clock
            put<FF_RNG_SEED>(get<FF_RNG_SEED>(ea), i); // save the seed that was used to evaluate this individual
            calculate_fitness(i, typename EA::encoding_tag(), rng, ea);
        }        
        
        //! Constant: calculate fitness only if this individual has not yet been evaluated.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, constantS, EA& ea) {
            if(ea::fitness(i).is_null()) {
                calculate_fitness(i, typename EA::fitness_function_type::stability_tag(), ea);
            }
        }
        
        //! Nonstationary: always calculate fitness.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, nonstationaryS, EA& ea) {
            calculate_fitness(i, typename EA::fitness_function_type::stability_tag(), ea);
        }
        
        //! Absolute: do nothing.
        template <typename ForwardIterator, typename EA>
        void relativize_fitness(ForwardIterator first, ForwardIterator last, absoluteS, EA& ea) {
        }
        
        //! Relative: relativize(?) fitness.
        template <typename ForwardIterator, typename EA>
        void relativize_fitness(ForwardIterator first, ForwardIterator last, relativeS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            ea.relativize(first, last);
        }
    }

    /*! Calculate the fitness of an individual, respecting various fitness function tags.
     */
    template <typename EA>
    void calculate_fitness(typename EA::individual_type& i, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        detail::calculate_fitness(i, typename EA::fitness_function_type::constant_tag(), ea);
    }
    
    /*! Calculate fitness for the range [f,l).
	 */
	template <typename ForwardIterator, typename EA>
	void calculate_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		for(; first!=last; ++first) {
			calculate_fitness(**first,ea);
		}
	}

    /*! Nullify fitness for the range [f,l).
	 */
	template <typename ForwardIterator, typename EA>
	void nullify_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		for(; first!=last; ++first) {
            ea::fitness(**first).nullify();
		}
	}

    /*! Unconditionally recalculate fitness for the range [f,l).
     */
	template <typename ForwardIterator, typename EA>
	void recalculate_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		for(; first!=last; ++first) {
            ea::fitness(**first).nullify();
			calculate_fitness(**first,ea);
		}
    }    
    
    /*! Calculate relative fitness for the range [f,l).
	 */
	template <typename ForwardIterator, typename EA>
	void relativize_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        detail::relativize_fitness(first, last, typename EA::fitness_function_type::relative_tag(), ea);
	}

    /*! Accumulated the fitness for a range of individuals, calculating it if needed.
     */
    template <typename ForwardIterator, typename EA>
    typename EA::fitness_type accumulate_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        typename EA::fitness_type sum=0.0;
        for(; first!=last; ++first) {
            calculate_fitness(ind(first,ea),ea);
            sum += ind(first,ea).fitness();
        }
        return sum;
    }
    
    
    /*! This event periodically (re-)initializes the fitness function for the 
     entire population.
     
     Note that this triggers a fitness reevaluation for all individuals in
     the population.
     
     Also note that it is the fitness function's responsibility to store whatever
     information is needed to recreate the correct landscape for serialization.
     */
    template <typename EA>
    struct reinitialize_fitness_function : periodic_event<FF_INITIALIZATION_PERIOD, EA> {
        reinitialize_fitness_function(EA& ea) : periodic_event<FF_INITIALIZATION_PERIOD, EA>(ea) {
        }
        
        virtual void operator()(EA& ea) {
            initialize_fitness_function(ea.fitness_function(), ea);
            recalculate_fitness(ea.population().begin(), ea.population().end(), ea);
        }
    };
    
} // ea

#endif
