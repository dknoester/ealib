#ifndef _EA_FITNESS_FUNCTION_H_
#define _EA_FITNESS_FUNCTION_H_

#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <limits>
#include <string>
#include <vector>
#include <ea/meta_data.h>

namespace ea {
    
    //! Indicates that fitness of an individual is absolute.
    struct absoluteS { };

    //! Indicates that fitness of an individual is relative to the population.
    struct relativeS { };
    
    //! Indicates that fitness is stationary, and thus should be cached.
    struct stableS { };

    //! Indicates that fitness may change between evalutions, and should not be cached.
    struct nonstationaryS { };

    //! Indicates that fitness is stable, and does not require its own RNG.
    struct constantS { };

    //! Indicates that fitness is stochastic, and requires its own RNG.
    struct stochasticS { };

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
        
        //! Serialize this fitness value.
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) { 
            ar & boost::serialization::make_nvp("f", _f);
        }
        
        value_type _f; //!< Fitness value.
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
        
        objective_type operator[](std::size_t i) { return _f[i]; }

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
//            ar & boost::serialization::make_nvp("f", _f);
        }
        
        value_type _f; //!< Fitness value.
    };


    /*! Convenience struct to define the needed typedefs for a unary fitness
     function object.
     */
    template <typename T, 
    typename ConstantTag=constantS,
    typename StabilityTag=stableS,
    typename RelativeTag=absoluteS>
    struct fitness_function {
        typedef T fitness_type;
        typedef typename fitness_type::value_type value_type;
        typedef RelativeTag relative_tag;
        typedef StabilityTag stability_tag;
        typedef ConstantTag constant_tag;
        
        template <typename EA>
        void initialize(EA& ea) {
        }
        
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) { 
        }
    };
    
    
    namespace detail {
        //! Constant: evaluate fitness without an embedded RNG.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, constantS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            i.fitness() = ea.fitness_function()(i,ea);
            ea.events().fitness_evaluated(i,ea);
        }
        
        //! Stochastic: provide an RNG for use by the fitness function.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, stochasticS, EA& ea) {
            BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
            
            next<FF_RNG_SEED>(ea);
            typename EA::rng_type rng(get<FF_RNG_SEED>(ea)+1); // +1 to avoid clock
            put<FF_RNG_SEED>(get<FF_RNG_SEED>(ea), i); // save the seed that was used to evaluate this individual

            i.fitness() = ea.fitness_function()(i,rng,ea);
            ea.events().fitness_evaluated(i,ea);
        }        
        
        //! Stable: calculate fitness only if this individual has not yet been evaluated.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, stableS, EA& ea) {
            if(i.fitness().is_null()) {
                calculate_fitness(i, typename EA::fitness_function_type::constant_tag(), ea);
            }
        }
        
        //! Nonstationary: always calculate fitness.
        template <typename EA>
        void calculate_fitness(typename EA::individual_type& i, nonstationaryS, EA& ea) {
            calculate_fitness(i, typename EA::fitness_function_type::constant_tag(), ea);
        }
    }
    
    /*! Calculate the fitness of an individual, respecting various fitness function tags.
     */
    template <typename EA>
    void calculate_fitness(typename EA::individual_type& i, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        detail::calculate_fitness(i, typename EA::fitness_function_type::stability_tag(), ea);
    }
    
    /*! Calculate fitness for the range [f,l).
	 */
	template <typename ForwardIterator, typename EA>
	void calculate_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
		BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
		for(; first!=last; ++first) {
			calculate_fitness(ind(first,ea),ea);
		}
	}

    /*! Accumulated the fitness for a range of individuals, calculating it if needed.
     */
    template <typename ForwardIterator, typename EA>
    typename EA::fitness_type accumulate_fitness(ForwardIterator first, ForwardIterator last, EA& ea) {
        BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
        typename EA::fitness_value_type sum=0.0;
        for(; first!=last; ++first) {
            calculate_fitness(ind(first,ea),ea);
            sum += ind(first,ea).fitness();
        }
        return sum;
    }
    
//    /*! Adaptor for stochastic fitness functions.
//     */
//    template <typename FitnessFunction>
//    struct stochastic : fitness_function<typename FitnessFunction::value_type> {
//        typedef FitnessFunction fitness_function_type;
//        typedef typename fitness_function_type::value_type value_type;
//        
//        template <typename EA>
//        void initialize(EA& ea) {
//            next<FF_RNG_SEED>(ea);
//            typename EA::rng_type rng(get<FF_RNG_SEED>(ea)+1); // +1 to avoid clock
//            _ff.initialize(rng, ea);
//        }
//
//        template <typename EA>
//        void reinitialize(EA& ea) {
//            initialize(ea);
//        }
//
//        template <typename Individual, typename EA>
//        value_type operator()(Individual& ind, EA& ea) {
//            return _ff(ind,ea);
//        }
//        
//        fitness_function_type _ff; //!< Fitness function.
//    };
////    
//    
//    /*! Adaptor for alternating among a series of fitness functions.
//     */
//    template <typename FitnessFunction>
//    struct switching : fitness_function<typename FitnessFunction::value_type> {
//        typedef FitnessFunction fitness_function_type;
//        typedef typename fitness_function_type::value_type value_type;
//        typedef std::vector<fitness_function_type> ff_list_type;        
//        
//        template <typename EA>
//        void initialize(EA& ea) {
//            fitness_functions.resize(get<FF_N>(ea));
//            for(typename ff_list_type::iterator i=fitness_functions.begin(); i!=fitness_functions.end(); ++i) {
//                i->initialize(ea);
//            }
//            _current = 0;
//        }
//
//        template <typename EA>
//        void reinitialize(EA& ea) {
//            _current = ++_current % fitness_functions.size();
//        }
//
//        template <typename Individual, typename EA>
//        value_type operator()(Individual& ind, EA& ea) {
//            return fitness_functions[_current](ind,ea);
//        }
//        
//        ff_list_type fitness_functions; //!< All fitness functions.
//        std::size_t _current; //!< Index of the currently active fitness function.
//    };
    
} // ea

#endif
