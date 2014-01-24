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
#include <boost/serialization/vector.hpp>
#include <limits>
#include <string>
#include <vector>
#include <ea/algorithm.h>
#include <ea/concepts.h>
#include <ea/meta_data.h>
#include <ea/events.h>

namespace ealib {

    LIBEA_MD_DECL(FF_RNG_SEED, "ea.fitness_function.rng_seed", int);
    LIBEA_MD_DECL(FF_INITIAL_RNG_SEED, "ea.fitness_function.initial_rng_seed", int);
    LIBEA_MD_DECL(FF_INITIALIZATION_PERIOD, "ea.fitness_function.initialization_period", int);
    
    /* The following are tags that are to be used to indicate properties of a
     given fitness function:
     */
    
    //! Indicates that fitness is constant, and can be cached.
    struct constantS { };
    
    //! Indicates that fitness may change between evaluations, and should not be cached.
    struct nonstationaryS { };
    
    //! Indicates that fitness is deterministic, and does not require its own RNG.
    struct deterministicS { };

    //! Indicates that fitness is stochastic, and requires its own RNG.
    struct stochasticS { };

    //! Indicates that fitness should be maximized.
    struct maximizeS { };
    
    //! Indicates that fitness should be minimized.
    struct minimizeS { };

    /* While fitnesses are typically thought of as a single real value (e.g., a
     double), treating them as an object has numerous benefits, especially for
     multiobjective problems.  Here is the unary fitness value:
     */
    
    /*! Unary fitness value.
     */
    template <typename T,
    typename DirectionTag=maximizeS>
    struct unary_fitness {
        typedef T value_type;
        typedef DirectionTag direction_tag;

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

        //! Operator==
        bool operator==(const unary_fitness& that) const {
            if(is_null()) {
                return that.is_null();
            } else {
                return _f == that._f;
            }
        }

        //! Operator==
        bool operator==(const value_type v) const {
            if(is_null()) {
                return std::isnan(v);
            } else {
                return _f == v;
            }
        }

        /*! Operator <.
         
         The semantics of comparisons depend on whether fitness is being maximized
         or minimized.  Let A and B be fitnesses.  Then, if A<B, A has "better"
         fitness than B.  If we're maximizing, then A has numerically larger fitness,
         and vice versa.
         */
        bool operator<(const unary_fitness& that) const {
            return lt(that._f, direction_tag());
        }
        
        //! Operator>; see above.
        bool operator>(const unary_fitness& that) const {
            return gt(that._f,direction_tag());
        }

        //! Operator<=; see above.
        bool operator<=(const unary_fitness& that) const {
            return operator<(that) || operator==(that);
        }
        
        //! Operator>=; see above.
        bool operator>=(const unary_fitness& that) const {
            return operator>(that) || operator==(that);
        }

        //! Operator<; see above.
        bool operator<(const value_type f) const {
            return lt(f, direction_tag());
        }
        
        //! Operator>; see above.
        bool operator>(const value_type f) const {
            return gt(f,direction_tag());
        }
        
        //! Operator<=; see above.
        bool operator<=(const value_type f) const {
            return operator<(f) || operator==(f);
        }
        
        //! Operator>=; see above.
        bool operator>=(const value_type f) const {
            return operator>(f) || operator==(f);
        }

        //! Returns true if this maximized fitness is greater than that fitness.
        bool gt(const value_type f, maximizeS) const {
            return _f > f;
        }
        
        //! Returns true if this minimized fitness is greater than that fitness.
        bool gt(const value_type f, minimizeS) const {
            return _f < f;
        }

        //! Returns true if this maximized fitness is less than that fitness.
        bool lt(const value_type f, maximizeS) const {
            return _f < f;
        }
        
        //! Returns true if this minimized fitness is less than that fitness.
        bool lt(const value_type f, minimizeS) const {
            return _f > f;
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
            int null_fitness=is_null();
			ar & BOOST_SERIALIZATION_NVP(null_fitness);
			if(!is_null()) {
				ar & boost::serialization::make_nvp("value_type", _f);
			}
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            int null_fitness=1;
			ar & BOOST_SERIALIZATION_NVP(null_fitness);
			if(null_fitness) {
                nullify();
			} else {
				ar & boost::serialization::make_nvp("value_type", _f);
			}
		}
        
        value_type _f; //!< Fitness value.
        
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };
    
    
    /*! Multivalued fitness object.
     */
    template <typename T,
    typename DirectionTag=maximizeS>
    struct multivalued_fitness {
        typedef unary_fitness<T,DirectionTag> objective_type;
        typedef DirectionTag direction_tag;
        typedef std::vector<objective_type> value_type;
        
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
            ar & boost::serialization::make_nvp("value_type", _f);
        }
        
        value_type _f; //!< Fitness value.
    };

    /*! Convenience struct to define typedefs and empty initialization and serialization
     methods for fitness function objects.
     */
    template <typename T,
    typename ConstantTag=constantS,
    typename StabilityTag=deterministicS>
    struct fitness_function {
        typedef T fitness_type;
        typedef typename fitness_type::value_type value_type;
        typedef typename fitness_type::direction_tag direction_tag;
        typedef ConstantTag constant_tag;
        typedef StabilityTag stability_tag;
        
        //! Initialize this (deterministic) fitness function.
        template <typename EA>
        void initialize(EA& ea) {
        }
        
        //! Initialize this (stochastic) fitness function.
        template <typename RNG, typename EA>
        void initialize(RNG& rng, EA& ea) {
        }
        
        //! Number of objectives associated with this fitness function.
        std::size_t size() const {
            return 1;
        }
    };
    
    
    namespace detail {
        //! Deterministic initialization (no RNG needed).
        template <typename FitnessFunction, typename EA>
        void initialize_fitness_function(FitnessFunction& ff, deterministicS, EA& ea) {
            ff.initialize(ea);
        }
        
        //! Stochastic initialization (RNG needed).
        template <typename FitnessFunction, typename EA>
        void initialize_fitness_function(FitnessFunction& ff, stochasticS, EA& ea) {
            ealib::next<FF_INITIAL_RNG_SEED>(ea);
            typename EA::rng_type rng(get<FF_INITIAL_RNG_SEED>(ea)+1); // +1 to avoid clock
            ff.initialize(rng, ea);
        }
    } // detail
    
    //! Initialize the fitness function; called prior to any fitness evaluation, but after meta-data.
    template <typename FitnessFunction, typename EA>
    void initialize_fitness_function(FitnessFunction& ff, EA& ea) {
        detail::initialize_fitness_function(ff, typename FitnessFunction::stability_tag(), ea);
    }

    
    namespace detail {
        //! Deterministic: evaluate fitness without an embedded RNG.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, deterministicS, EA& ea) {
            i.fitness() = ea.fitness_function()(i, ea);
            ea.events().fitness_evaluated(i,ea);
        }
        
        //! Stochastic: provide an RNG for use during fitness evaluation.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, stochasticS, EA& ea) {
            int seed = ea.rng().seed();
            typename EA::rng_type rng(seed);
            put<FF_RNG_SEED>(seed, i); // save the seed that was used to evaluate this individual
            i.fitness() = ea.fitness_function()(i, rng, ea);
            ea.events().fitness_evaluated(i,ea);
        }
        
        //! Constant: calculate fitness only if this individual has not yet been evaluated.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, constantS, EA& ea) {
            if(i.fitness().is_null()) {
                calculate_fitness(i, typename EA::fitness_function_type::stability_tag(), ea);
            }
        }
        
        //! Nonstationary: always calculate fitness.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, nonstationaryS, EA& ea) {
            calculate_fitness(i, typename EA::fitness_function_type::stability_tag(), ea);
        }
    } // detail
    
    //! Calculate the fitness of an individual.
    template <typename EA>
    void calculate_fitness(typename EA::individual_type& i, EA& ea) {
        detail::calculate_fitness(i, typename EA::fitness_function_type::constant_tag(), ea);
    }
    
    //! Calculate fitness for the range [f,l).
	template <typename ForwardIterator, typename EA>
	void calculate_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		for(; first!=last; ++first) {
			calculate_fitness(**first,ea);
		}
	}

    //! Fitness trait accessor (may calculate if null).
    template <typename EA>
    typename EA::individual_type::fitness_type& fitness(typename EA::individual_type& ind, EA& ea) {
        detail::calculate_fitness(ind, typename EA::fitness_function_type::constant_tag(), ea);
        return ind.fitness();
    }
    
    //! Returns true if the individual has a valid fitness.
    template <typename EA>
    bool has_fitness(typename EA::individual_type& i, EA& ea) {
        return !i.fitness().is_null();
    }

    //! Nullify the fitness of an individual.
    template <typename EA>
    void nullify_fitness(typename EA::individual_type& ind, EA& ea) {
        ind.fitness().nullify();
    }

    //! Nullify fitness for the population range [f,l).
	template <typename ForwardIterator, typename EA>
	void nullify_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		for(; first!=last; ++first) {
            first->fitness().nullify();
		}
	}

    //! Unconditionally recalculate fitness for the range [f,l).
	template <typename ForwardIterator, typename EA>
	void recalculate_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		for(; first!=last; ++first) {
            nullify_fitness(**first,ea);
			calculate_fitness(**first,ea);
		}
    }
    
} // ealib

#endif
