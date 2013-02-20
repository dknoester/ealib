/* sigmoid.h
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
#ifndef _ANN_SIGMOID_H_
#define _ANN_SIGMOID_H_

#include <cmath>
#include <functional>

namespace ann {

    template <typename T>
    struct step : std::unary_function<T,T> {
        typedef std::unary_function<T,T> base_type;
        typedef typename base_type::argument_type argument_type;
        typedef typename base_type::result_type result_type;

        step(T lv, T ip, T uv) : _lv(lv), _ip(ip), _uv(uv) { }

        result_type operator()(argument_type x) {
            if(x > _ip) {
                return _uv;
            } else {
                return _lv;
            }
        }
        
        T _lv, _ip, _uv; //!< lower value, inflection point, upper value
    };

    template <typename T>
    struct clip : std::unary_function<T,T> {
        typedef std::unary_function<T,T> base_type;
        typedef typename base_type::argument_type argument_type;
        typedef typename base_type::result_type result_type;
        
        clip(T lt, T lv, T ut, T uv) : _lt(lt), _lv(lv), _ut(ut), _uv(uv) {
        }
        
        result_type operator()(argument_type x) {
            if(x >= _ut) {
                return _uv;
            } else if(x <= _lt) {
                return _lv;
            } else {
                return x;
            }
        }

        T _lt, _lv, _ut, _uv; //!< lower and upper thresholds and values.
    };

    template <typename T>
    struct identity : std::unary_function<T,T> {
        typedef std::unary_function<T,T> base_type;
        typedef typename base_type::argument_type argument_type;
        typedef typename base_type::result_type result_type;
      
        result_type operator()(argument_type x) {
            return x;
        }
    };
    
    
	/*! Heaviside function (aka unit step), a binary activation sigmoid type.
     
     Domain: [-1.0, 1.0]
     Range: {0.0, 1.0}

     \warning The definition of H(0) can be significant; this was not selected
     with care.
	 */
	struct heaviside {
		//! Constructor.
		heaviside() {
		}
		
		//! Calculate the heaviside function of x.
		double operator()(double x) {
			return (x <= 0.0) ? 0.0 : 1.0;
		}

		// it's not clear what the derivative is here...
        //		//! Calculate the derivative of the logistic sigmoid of x.
        //		double derivative(double x) {
        //		}	
	};

    
	/*! Logistic function, a type of sigmoid.
     
     Domain: [-1.0, 1.0]
     Range: [0.0, 1.0]
     
     Lambda was selected to provides a nice sigmoid over the full domain.
	 */
	struct logistic {
		//! Constructor.
		logistic(double l=6.0) : lambda(l) {
		}
		
		//! Calculate the logistic sigmoid of x.
		double operator()(double x) {
			return 1/(1+exp(-lambda*x));
		}
		
		//! Calculate the derivative of the logistic sigmoid of x.
		double derivative(double x) {
			double s = operator()(x);
			return s * (1 - s);
		}	
		
		double lambda; //!< Lambda; steepens the gradient of the sigmoid.
	};
	
	
	/*! Hyperbolic tangent function, a type of sigmoid.
     
     Domain: [-1.0, 1.0]
     Range: [-1.0, 1.0]
     
     Lambda was selected to provides a nice sigmoid over the full domain.
	 */
	struct hyperbolic_tangent {
		//! Constructor.
		hyperbolic_tangent(double l=3.0) : _lambda(l) { }
		
		//! Calculate the hyperbolic tangent of x.
		double operator()(double x) {
			return tanh(_lambda*x);
		}
		
		//! Calculate the derivative of the hyperbolic tangent of x.
		double derivative(double x) {
			return 1-pow(operator()(x), 2);
		}
        
        double _lambda; //!< Used to steepen the gradient of the sigmoid.
	};
	
} // ann

#endif
