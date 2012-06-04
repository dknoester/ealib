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
