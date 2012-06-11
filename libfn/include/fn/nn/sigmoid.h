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
#ifndef _NN_SIGMOID_H_
#define _NN_SIGMOID_H_

#include <cmath>

namespace nn {
	
	/*! Logistic function, a type of sigmoid.
	 */
	struct logistic_function {
		//! Constructor.
		logistic_function(double l=1.0) : lambda(l) {
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
	 */
	struct hyperbolic_tangent {
		//! Constructor.
		hyperbolic_tangent() { }
		
		//! Calculate the hyperbolic tangent of x.
		double operator()(double x) {
			return tanh(x);
		}
		
		//! Calculate the derivative of the hyperbolic tangent of x.
		double derivative(double x) {
			return 1-pow(operator()(x), 2);
		}
	};
	
} // nn

#endif
