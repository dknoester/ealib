/* rng.h
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
#ifndef _EA_RNG_H_
#define _EA_RNG_H_

#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iterator>
#include <vector>
#include <limits>
#include <set>
#include <sstream>
#include <ctime>

#include <ea/algorithm.h>
#include <ea/metadata.h>


namespace ealib {
    
    LIBEA_MD_DECL(RNG_SEED, "ea.rng.seed", unsigned int);

	/*! Provides useful abstractions for dealing with random numbers.
	 
	 Note: when many random numbers are needed, consider using the uniform_X_rng
	 methods.  These methods return a generator that can be quickly queried for new 
	 random numbers, as opposed to the uniform_X methods, that build a new generator 
	 and use it once for each call.
	 */
	template <typename Engine>
	class rng {
	public:
		//! Underlying source of randomness.
		typedef Engine engine_type;
		//! Uniform real distribution.
		typedef boost::uniform_real< > uniform_real_dist;
		//! Uniform integer distribution.
		typedef boost::uniform_int< > uniform_int_dist;
		//! Generator for uniformly-distributed random real numbers.
		typedef boost::variate_generator<engine_type&, uniform_real_dist> real_rng_type;
		//! Generator for uniformly-distributed random integers.
		typedef boost::variate_generator<engine_type&, uniform_int_dist> int_rng_type;
		//! Generator for UUIDs.
		typedef boost::uuids::basic_random_generator<engine_type> uuid_generator_type;
		//! Normal real distribution.
		typedef boost::normal_distribution< > normal_real_dist;
		//! Generator for normally-distributed random real numbers.
		typedef boost::variate_generator<engine_type&, normal_real_dist> normal_real_rng_type;
		//! Normal integer distribution.
		typedef boost::normal_distribution<int> normal_int_dist;
		//! Generator for normally-distributed random integers.
		typedef boost::variate_generator<engine_type&, normal_int_dist> normal_int_rng_type;
		//! Type of the argument to this RNG (to support concept requirements).
		typedef int argument_type;
		//! Type returned by this RNG (to support concept requirements).
		typedef int result_type;

		//! Constructor.
		rng() {
			reset(static_cast<unsigned int>(std::time(0)));
		}
		
		//! Constructor with specified rng seed.
		rng(unsigned int s) {
			reset(s);
		}
		
        //! Copy constructor.
		rng(const rng& that) : _eng(that._eng) {
            _p.reset(new real_rng_type(_eng, uniform_real_dist(0.0,1.0)));
			_bit.reset(new int_rng_type(_eng, uniform_int_dist(0,1)));
			_uuid.reset(new uuid_generator_type(_eng));
        }
        
		//! Assignment operator.
		rng& operator=(const rng& that) {
            if(this != &that) {
                _eng = that._eng;
                _p.reset(new real_rng_type(_eng, uniform_real_dist(0.0,1.0)));
                _bit.reset(new int_rng_type(_eng, uniform_int_dist(0,1)));
				_uuid.reset(new uuid_generator_type(_eng));
			}
            return *this;
        }		

		//! Equivalence operator.
        bool operator==(const rng& that) {
            return _eng == that._eng;
        }
        
		//! Reset this random number generator with the specified seed.
		void reset(unsigned int s) {
			if(s == 0) {
				s = static_cast<unsigned int>(std::time(0));
			}
			_eng.seed(s);
			_p.reset(new real_rng_type(_eng, uniform_real_dist(0.0,1.0)));
			_bit.reset(new int_rng_type(_eng, uniform_int_dist(0,1)));
			_uuid.reset(new uuid_generator_type(_eng));
		}
		
		/*! Returns a random number in the range [0,n).
		 
		 This method enables this class to be used as a RandomNumberGenerator in
         STL algorithms.
		 */
		result_type operator()(argument_type n) {
			return uniform_integer_rng(0,n)();
		}

        /*! Returns a random number in the range [l,u).
		 
		 This method enables this class to be used as a RandomNumberGenerator in
         STL algorithms.
		 */
		result_type operator()(argument_type l, argument_type u) {
			return uniform_integer_rng(l,u)();
		}

		/*! Returns a random number in the range [0,maxint).
         */
        result_type operator()() {
			return uniform_integer_rng(0,std::numeric_limits<argument_type>::max())();
		}

        /*! Returns a random number in the range [1,maxint-1), suitable for generation of random number seeds.
         */
        result_type seed() {
            return 1 + uniform_integer_rng(0,std::numeric_limits<argument_type>::max()-1)();
		}

		/*! Test a probability.
		 
		 Returns true if P < prob, false if P >= prob.  Prob must be in the range [0,1].
		 */
		bool p(double prob) { assert((prob >= 0.0) && (prob <= 1.0)); return (*_p)() < prob; }

        //! Returns a probability.
        double p() { return (*_p)(); }
        
		//! Returns a random bit.
		bool bit() { return (*_bit)(); }
		
		//! Returns a random real value uniformly drawn from the range [min, max) 
		double uniform_real(double min, double max) { return uniform_real_rng(min,max)(); }

		//! Returns a random real value uniformly drawn from the range (min, max).
		double uniform_real_nz(double min, double max) {
            double r = uniform_real(min,max);
            while(r == 0.0) {
                r = uniform_real(min,max);
            }
            return r;
        }

		//! Returns a random number generator of reals over the range [min, max).
		real_rng_type uniform_real_rng(double min, double max) {
			return real_rng_type(_eng, uniform_real_dist(min,max));
		}

		//! Returns a random real value drawn from a normal distribution with the given mean and variance.
		double normal_real(double mean, double variance) {
			return normal_real_rng_type(_eng, normal_real_dist(mean, variance))();
		}
        
		//! Returns a generator of random real values drawn from a normal distribution with the given mean and variance.
		normal_real_rng_type normal_real_rng(double mean, double variance) {
			return normal_real_rng_type(_eng, normal_real_dist(mean, variance));
		}
		
		/*! Returns an integer value in the range [min, max).
		 
		 For consistency with most other random number generators, max will never be
		 returned.
		 */
		int uniform_integer(int min, int max) { return uniform_integer_rng(min,max)(); }
		
        /*! Returns a random integer.
         */
        int uniform_integer() { return uniform_integer(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()); }
        
		/*! Returns a random number generator of integers over the range [min, max).

		 For consistency with most other random number generators, max will never be
		 returned.  (Boost's integer rng allows max to be returned, thus the -1 below.)
		 */
		int_rng_type uniform_integer_rng(int min, int max) {
			return int_rng_type(_eng, uniform_int_dist(min,max-1));
		}

        /*! Generates random numbers into the given output iterator.
         */
        template <typename T, typename OutputIterator>
        void generate(std::size_t n, T min, T max, OutputIterator oi) {
            typedef std::set<int> set_type;
            set_type v;
            while(v.size() < n) {
                int i = uniform_integer(min, max);
                if(v.find(i) == v.end()) {
                    v.insert(i);
                    *oi++ = i;
                }
            }
        }
        
		/*! Returns a normally-distributed integer with the given mean and variance.
		 */
		int normal_int(int mean, int variance) {
			return static_cast<int>(normal_real(static_cast<double>(mean), static_cast<double>(variance))+0.5);
		}
		
		/*! Choose two different random numbers from [min,max), and return them in sorted order.
		 */
		template <typename T>
		std::pair<T,T> choose_two(T min, T max) {
			int_rng_type irng = uniform_integer_rng(static_cast<int>(min), static_cast<int>(max));
			int one=irng();	int two=irng();
			while(one == two) {
				two = irng();
			}
			if(one > two) { std::swap(one, two); }
			return std::make_pair(static_cast<T>(one), static_cast<T>(two));
		}
		
		/*! Choose two different random numbers from [min,max) and return them in arbitrary order.
		 */
		template <typename T>
		std::pair<T,T> choose_two_ns(T min, T max) {
			int_rng_type irng = uniform_integer_rng(static_cast<int>(min), static_cast<int>(max));
			int one=irng();	int two=irng();
			while(one == two) {
				two = irng();
			}
			return std::make_pair(static_cast<T>(one), static_cast<T>(two));
		}        

		/*! Choose two different iterators from the range [f,l), and return them in sorted order (r.first occurs before r.second).
		 */
		template <typename ForwardIterator>
		std::pair<ForwardIterator,ForwardIterator> choose_two_range(ForwardIterator f, ForwardIterator l) {
			int_rng_type irng = uniform_integer_rng(0, static_cast<int>(std::distance(f,l)));
			int one=irng();	int two=irng();
			while(one == two) {
				two = irng();
			}

			return std::make_pair(f+one, f+two);
		}

		/*! Sample elements uniformly with replacement from the given range, copying them to the output range.
		 */
		template <typename InputIterator, typename OutputIterator>
		void sample_with_replacement(InputIterator first, InputIterator last, OutputIterator output, std::size_t n) {
			std::size_t range = std::distance(first, last);
			int_rng_type irng = uniform_integer_rng(0,range);
			for( ; n>0; --n) {
                InputIterator t=first;
                std::advance(t,irng());
				*output++ = *t;
			}
		}

		/*! Sample n elements uniformly without replacement from [f,l), copying them to output.
		 */
		template <typename InputIterator, typename OutputIterator>
		void sample_without_replacement(InputIterator first, InputIterator last, OutputIterator output, std::size_t n) {
			typedef std::vector<std::size_t> replacement_type;
			std::size_t range = std::distance(first, last);
            assert(n <= range);
			
            replacement_type replace(range);
			algorithm::iota(replace.begin(), replace.end());
            std::random_shuffle(replace.begin(), replace.end(), *this);
            
			for(std::size_t i=0; n>0; --n, ++i) {
                *output++ = first[replace[i]];
			}
		}
		
		/*! Returns a randomly-selected iterator from the given range.
		 */
		template <typename InputIterator>
		InputIterator choice(InputIterator first, InputIterator last) {
			std::advance(first, uniform_integer(0, std::distance(first, last)));
            return first;
		}
		
		/*! Returns a randomly-selected iterator from the given range, selected without replacement.
		 
		 The passed-in replacement map is used to ensure that no single element from [first,last) is
		 returned more than once.  If an empty replacement map is passed-in, it will be initialized,
		 as an empty map would indicate that the entire range was selected.
		 
		 //! Replacement map, used to track elements that were selected for a range for without-replacment methods.
		 typedef std::vector<std::size_t> replacement_map_type;
		 */
		template <typename InputIterator, typename ReplacementMap>
		InputIterator choice(InputIterator first, InputIterator last, ReplacementMap& rm) {
			if(rm.empty()) {
				rm.resize(std::distance(first,last));
				iota(rm.begin(), rm.end(), 0);
			}
			
			typename ReplacementMap::iterator i=rm.begin();
			std::advance(i, uniform_integer(0, rm.size()));
			std::advance(first, *i);
			rm.erase(i);
			return first;
		}
		
		//! Returns a random UUID in string format.
		std::string uuid() {
			return boost::lexical_cast<std::string>((*_uuid)());
		}

	private:
		engine_type _eng; //!< Underlying generator of randomness.
		boost::scoped_ptr<real_rng_type> _p; //!< Generator for probabilities.
		boost::scoped_ptr<int_rng_type> _bit; //!< Generator for bits.
		boost::scoped_ptr<uuid_generator_type> _uuid; //!< Generator for UUIDs.

		// These enable serialization and de-serialization of the rng state.
		friend class boost::serialization::access;
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
			std::ostringstream out;
			out << _eng;
			std::string state(out.str());
			ar & BOOST_SERIALIZATION_NVP(state);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
			std::string state;
			ar & BOOST_SERIALIZATION_NVP(state);
			std::istringstream in(state);
			in >> _eng;
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
	};
    
    /*! STL-compatible generator for probabilities.
     */
    template <typename RNG>
    struct probability_generator {
        typedef double result_type;
        probability_generator(RNG& rng) : _rng(rng) { }
        result_type operator()() { return _rng.p(); }
        RNG& _rng;
    };

    /*! STL-compatible generator for integers.
     */
    template <typename RNG>
    struct integer_generator {
        typedef int result_type;
        integer_generator(RNG& rng) : _rng(rng) { }
        result_type operator()() { return _rng.uniform_integer(); }
        RNG& _rng;
    };

    /*! STL-compatible generator for bits.
     */
    template <typename RNG>
    struct bit_generator {
        typedef int result_type;
        bit_generator(RNG& rng) : _rng(rng) { }
        result_type operator()() { return _rng.bit(); }
        RNG& _rng;
    };

	//! Default random number generation type.
	typedef rng<boost::mt19937> default_rng_type;

} // util

#endif
