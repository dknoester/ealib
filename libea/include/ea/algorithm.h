#ifndef _EA_ALGORITHM_H_
#define _EA_ALGORITHM_H_

#include <algorithm>
#include <boost/lexical_cast.hpp>

namespace ea {
	namespace algorithm {
        
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
        
        template <typename T>
        T clip(T value, T min, T max) {
            return std::max(min, std::min(value, max));
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
            for( ; f!=l; ++f) {
                sum += *f;
            }
            return sum / static_cast<T>(c);
        }

	} // algorithm
} // ea

#endif
