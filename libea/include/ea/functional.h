/* functional.h
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
#ifndef _EA_FUNCTIONAL_H_
#define _EA_FUNCTIONAL_H_

#include <functional>

namespace ealib {
    
    template <typename T>
    struct binary_or : std::binary_function<T,T,T> {
        typedef std::binary_function<T,T,T> parent;
        typedef typename parent::first_argument_type first_argument_type;
        typedef typename parent::second_argument_type second_argument_type;
        typedef typename parent::result_type result_type;
        
        result_type operator()(first_argument_type x, second_argument_type y) {
            return x | y;
        }
    };

    template <typename T>
    struct non_zero : std::unary_function<T,int> {
        typedef std::unary_function<T,T> parent;
        typedef typename parent::argument_type argument_type;
        typedef typename parent::result_type result_type;
        
        int operator()(argument_type x) {
            if(x != 0) {
                return 0x01;
            } else {
                return 0x0;
            }
        }
    };


    /*! Compose two adaptable unary functions f and g into a single unary function
     h such that h(x) = f(g(x)).
     */
    template <typename AdaptableUnaryFunction1, typename AdaptableUnaryFunction2>
    struct unary_compose {
        typedef typename AdaptableUnaryFunction2::argument_type argument_type;
        typedef typename AdaptableUnaryFunction1::result_type result_type;
        
        //! Default constructor.
        unary_compose() {
        }
        
        //! Constructor.
        unary_compose(const AdaptableUnaryFunction1& f, const AdaptableUnaryFunction2& g)
        : _f(f), _g(g) {
        }
        
        //! Return f(g(x)).
        result_type operator()(const argument_type& x) {
            return _f(_g(x));
        }
        
        AdaptableUnaryFunction1 _f; //!< f(x).
        AdaptableUnaryFunction2 _g; //!< g(x).
    };
    
    //! Convenience method to compose f(x) and g(x) into h(x) = f(g(x)).
    template <class AdaptableUnaryFunction1, class AdaptableUnaryFunction2>
    unary_compose<AdaptableUnaryFunction1, AdaptableUnaryFunction2>
    compose1(const AdaptableUnaryFunction1& f,
             const AdaptableUnaryFunction2& g) {
        return unary_compose<AdaptableUnaryFunction1, AdaptableUnaryFunction2>(f,g);
    }
    
} // ea

#endif
