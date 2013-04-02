/* algorithm.h 
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

#ifndef _EA_ALGORITHM_H_
#define _EA_ALGORITHM_H_

#include <algorithm>
#include <numeric>
#include <boost/lexical_cast.hpp>

namespace ealib {
	namespace algorithm {
        
        template <typename ForwardIterator>
        unsigned int hamming_distance(ForwardIterator f1, ForwardIterator l1, ForwardIterator f2) {
            unsigned int d=0;
            for( ; f1!=l1; ++f1, ++f2) {
                if((*f1) != (*f2)) {
                    ++d;
                }
            }
            return d;
        }
        
        struct hamming_distance_functor {
            template <typename Individual, typename EA>
            unsigned int operator()(Individual& i1, Individual& i2, EA& ea) {
                return hamming_distance(i1.repr().begin(), i1.repr().end(), i2.repr().begin());
            }
        };
        
        /*! Roulette wheel selection.
		 
		 Returns the index and iterator selected from the range [f,l), based on the sum of values
		 of the items in the sequence.
		 
		 If a value could not be found, return l.
		 */
		template <typename ForwardIterator>
        std::pair<std::size_t, ForwardIterator> roulette_wheel(const typename ForwardIterator::value_type& target, ForwardIterator f, ForwardIterator l) {
			typename ForwardIterator::value_type running=0.0;
            std::size_t i=0;
			for( ; f!=l; ++f, ++i) {
				running += *f;
				if(running >= target) {
					return std::make_pair(i,f);
				}
			}
            // if we get here, then target > sum(f,l); this is either a floating point
            // or logical error.  we'll return the final index and l; be warned.
			return std::make_pair(i-1,l);
		}
        
        /*! Assign sequentially increasing values to a range.
		 */
		template <typename ForwardIterator, typename T>
		void iota(ForwardIterator first, ForwardIterator last, T initial, T delta) {
			for(; first != last; ++first, initial+=delta)
				*first = initial;
		}
		
		
		/*! Assign sequentially increasing unsigned integers starting from 0 to a range.
		 */
		template <typename ForwardIterator>
		void iota(ForwardIterator first, ForwardIterator last) {
			for(std::size_t i=0; first != last; ++first, ++i)
				*first = i;
		}
        
        //! x == [-x_range,x_range], normalized to [-output_range,output_range]
        template <typename T>
        T normalize(T x, T x_range, T output_range) {
            x = std::min(x, x_range);
            x = std::max(x, -x_range);
            return output_range * x / x_range;
        }
        
        
        //! Normalize the range [f,l) to v, outputting results to o, and return an iterator to the end of the range.
        template <typename ForwardIterator, typename OutputIterator>
        ForwardIterator normalize(ForwardIterator f, ForwardIterator l, OutputIterator o, double v) {
            typedef typename OutputIterator::value_type value_type;
            
            double s=std::accumulate(f,l,0.0);
            double offset=0.0;
            if(s == 0.0) {
                s = 1.0;
                offset = 1.0 / static_cast<double>(std::distance(f,l));
            }
            for( ; f!=l; ++f) {
                *o++ = offset + static_cast<double>(*f) * v / s;
            }
            return f;
        }
        
        //! Normalize the range [f,l) to v in-place and return an iterator to the end of the range.
        template <typename ForwardIterator>
        ForwardIterator normalize(ForwardIterator f, ForwardIterator l, double v) {
            return normalize(f, l, f, v);
        }

        template <typename T>
        T clip(T value, T min, T max) {
            return std::max(min, std::min(value, max));
        }
        
        template <typename T>
        T roll(T value, T min, T max) {
            if(value>max) {
                return min;
            } else if(value<min) {
                return max;
            }
            return value;
        }
        
        template <typename T>
        T modnorm(T x, T floor, T limit) {
            if((limit-floor) == 0) {
                return floor;
            }
            return (x%(limit-floor)) + floor;
        }
        
        //! Convert a sequence of values to a single string with the given separator.
        template <typename ForwardIterator>
        std::string vcat(ForwardIterator f, ForwardIterator l, const char* sep=" ") {
            std::string s;
            if(f != l) {
                s += boost::lexical_cast<std::string>(*f);
                ++f;
                for( ; f!=l; ++f) {
                    s += sep;
                    s += boost::lexical_cast<std::string>(*f);
                }
            }
            return s;
        }
        
        //! Perform a indexing operation of the given vector.
        template <typename Vector, typename Indices>
        Vector vindex(const Vector& v, const Indices& idx) {
            Vector r;
            for(std::size_t i=0; i<idx.size(); ++i) {
                r.push_back(v[idx[i]]);
            }
            return r;
        }
        
        //! Calculates Euclidean distance from (0...) for the points in [f,l).
        template <typename ForwardIterator>
        double vmag(ForwardIterator f, ForwardIterator l) {
            double ss=0.0;
            for( ; f!=l; ++f) {
                ss += (*f) * (*f);
            }
            return sqrt(ss);
        }
        
        //! Calculates Euclidean distance between two points of arbitrary dimension. Assumes points are of same dimension.
        template <typename ForwardIterator>
        double vdist(ForwardIterator af, ForwardIterator al, ForwardIterator bf, ForwardIterator bl) {
            double ss = 0.0;
            for( ; af != al && bf != bl; ++af, ++bf) {
                double diff = (*af) - (*bf);
                ss += diff * diff;
            }
            return sqrt(ss);
        }
        
        //! Calculate the cumulative XOR of [f,l).
        template <typename ForwardIterator>
        int vxor(ForwardIterator f, ForwardIterator l) {
            int result = *f;
            for( ; f!=l; ++f) {
                result = result ^ *f;
            }
            return result;
        }
        
        //! Calculates the mean of the values between [f, l).
        template <typename ForwardIterator, typename T>
        T vmean(ForwardIterator f, ForwardIterator l, T init) {
            T sum=init;
            std::size_t c=0;
            for( ; f!=l; ++f, ++c) {
                sum += *f;
            }
            return sum / static_cast<T>(c);
        }
        
        /*! Decode excitatory/inhibitory bit pairs into an integer.
         */
        template <typename ForwardIterator>
        int range_pair2int(ForwardIterator f, ForwardIterator l) {
            assert((std::distance(f,l)%2) == 0);
            int d=0;
            for(std::size_t j=0; f!=l; ++f, ++j) {
                int excite=(*f & 0x01);
                int inhibit=(*++f & 0x01);
                d |=  (excite & ~inhibit) << j;
            }
            return d;
        }
        
        /*! Decode excitatory/inhibitory bit pairs into a list of bits.
         */
        template <typename ForwardIterator, typename OutputIterator>
        void range_pair2int(ForwardIterator f, ForwardIterator l, OutputIterator oi) {
            assert((std::distance(f,l)%2) == 0);
            for( ; f!=l; ++f) {
                int excite=(*f & 0x01);
                int inhibit=(*++f & 0x01);
                *oi++ = (excite & ~inhibit);
            }
        }
        
        /*! Write the indices of "on" outputs to an output iterator.
         */
        template <typename ForwardIterator, typename OutputIterator>
        void range_pair2indices(ForwardIterator f, ForwardIterator l, OutputIterator oi) {
            assert((std::distance(f,l)%2) == 0);
            for(int i=0; f!=l; ++f, ++i) {
                int excite=(*f & 0x01);
                int inhibit=(*++f & 0x01);
                if(excite & ~inhibit) {
                    *oi++ = i;
                }
            }
        }

        /*! Write the indices of "on" outputs to an output iterator.
         */
        template <typename ForwardIterator, typename OutputIterator>
        void range2indices(ForwardIterator f, ForwardIterator l, OutputIterator oi) {
            for(int i=0; f!=l; ++f, ++i) {
                if(*f & 0x01) {
                    *oi++ = i;
                }
            }
        }

        /*! Convert the bits in range [f,l) to an integer.
         */
        template <typename ForwardIterator>
        int range2int(ForwardIterator f, ForwardIterator l) {
            int d=0;
            for(std::size_t j=0; f!=l; ++f, ++j) {
                d |=  (*f & 0x01) << j;
            }
            return d;
        }
        
	} // algorithm
} // ea

#endif
