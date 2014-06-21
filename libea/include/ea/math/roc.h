/* ocr_single.cpp
 * 
 * This file is part of OCR.
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
#ifndef _EA_MATH_ROC_H_
#define _EA_MATH_ROC_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>

/*! Receiver-Operating Characteristic statistics for binary classification.
 
 See: http://en.wikipedia.org/wiki/Receiver_operating_characteristic
 */
struct roc {
    typedef boost::numeric::ublas::matrix<double> matrix_type;
    typedef boost::numeric::ublas::matrix_column<matrix_type> column_type;
    typedef boost::numeric::ublas::matrix_row<matrix_type> row_type;
    
    //! Constructor.
    roc() : _M(3,2,0.0) {
    }
    
    //! Zero-out the ROC matrix.
    void clear() {
        _M = boost::numeric::ublas::zero_matrix<double>(3,2);
    }

    //! Update the ROC table with the condition and test outcome.
    void operator()(int condition, int test) {
        if(condition) {
            ++p();
            if(test) {
                ++tp();
            } else {
                ++fn();
            }
        } else {
            ++n();
            if(test) {
                ++fp();
            } else {
                ++tn();
            }
        }
    }

    //! Returns the number of positive conditions.
    double& p() { return _M(2,0); }

    //! Returns the number of negative conditions.
    double& n() { return _M(2,1); }
    
    //! Returns the number of true positives.
    double& tp() { return _M(0,0); }
    
    //! Returns the number of false positives.
    double& fp() { return _M(0,1); }
    
    //! Returns the number of false negatives.
    double& fn() { return _M(1,0); }
    
    //! Returns the number of true negatives.
    double& tn() { return _M(1,1); }

    //! Returns the true positive rate (eqv. with sensitivity, hit rate, recall).
    double tpr(std::size_t i) {
        return tp()/p();
    }
    
    //! Returns the true negative rate (eqv. with specificity).
    double tnr() {
        return tn()/n();
    }
    
    //! Returns the false positive rate (eqv. with fall-out).
    double fpr() {
        return fp()/n();
    }

    //! Returns the false negative rate (eqv. miss rate).
    double fnr() {
        return fn()/p();
    }

    //! Returns the positive predictive value (eqv. with precision).
    double ppv() {
        return tp()/(tp()+fp());
    }

    //! Returns the negative predictive value.
    double npv() {
        return tn()/(tn()+fn());
    }
    
    //! Returns the false discovery rate.
    double fdr() {
        return fp()/(fp()+tp());
    }

    //! Returns the accuracy.
    double acc() {
        return (tp()+tn())/(p()+n());
    }
    
    //! Returns the F1 score.
    double f1() {
        return (2.0*tp())/(2.0*tp()+fp()+fn());
    }

    //! Returns the Matthews correlation coeffiecient.
    double mcc() {
        double t = (tp()+fp()) * (tp()+fn()) * (tn()+fp()) * (tn()+fn());
        if(t == 0.0) {
            t = 1.0;
        }
        return (tp()*tn() - fp()*fn()) / sqrt(t);
    }

    matrix_type _M; //!< ROC matrix.
};

#endif
