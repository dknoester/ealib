/* statistics.h
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

#ifndef _EA_ANALYSIS_STATISTICS_H_
#define _EA_ANALYSIS_STATISTICS_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <map>
#include <ea/algorithm.h>

namespace ealib {
    namespace analysis {
        
        typedef boost::numeric::ublas::matrix<unsigned int> confusion_matrix_type;
        
        /*! Build an n-class confusion matrix from the sequences X (actual) and
         Y (predicted).
         
         Classes are assumed to be in the half-open range [0,n).
         
         A confusion matrix is an (m x n) matrix where m_{i,j} is the number of
         samples whose actual class==i and whose predicted class==j.
         
         In the case of a 2-class confusion matrix, TP etc are defined as:
         * C = [ TP, FP ]
         *     [ FN, TN ]
         */
        template <typename Sequence>
        confusion_matrix_type confusion_matrix(const Sequence& X, const Sequence& Y, std::size_t n) {
            assert(X.size() == Y.size());
            confusion_matrix_type C = boost::numeric::ublas::scalar_matrix<double>(n,n,0);
            
            for(std::size_t i=0; i<X.size(); ++i) {
                ++C(X[i],Y[i]);
            }
            return C;
        }
        
        /*! The Matthews correlation coefficient is a way to measure the correlation
         between two binary classifications.  It is generally regarded as the "best"
         way to measure correlation among classes of different size.
         
         It is calculated directly from the confusion matrix as:
         MCC = ((TP*TN)-(FP*FN)) / sqrt((TP+FP)*(TP+FN)*(TN+FP)*(TN+FN))
         
         The range of the MCC is [-1,1], where -1 represents complete disagreement,
         1 represents complete agreement, and 0 represents chance.
         */
        template <typename ConfusionMatrix>
        double matthews_correlation(const ConfusionMatrix& C) {
            assert(C.size1() == C.size2());
            assert(C.size1() == 2);
            double TP=C(0,0), TN=C(1,1), FP=C(0,1), FN=C(1,0);
            return ((TP*TN)-(FP*FN)) / sqrt((TP+FP)*(TP+FN)*(TN+FP)*(TN+FN));
        }
        
        typedef boost::numeric::ublas::matrix<double> frequency_matrix_type;
        
        /*! Calculates the frequence distribution of events in a matrix.
         
         All cells in M must be non-negative.
         */
        template <typename Matrix>
        frequency_matrix_type frequency_distribution(const Matrix& M) {
            double n=0;
            for(std::size_t i=0; i<M.size1(); ++i) {
                for(std::size_t j=0; j<M.size2(); ++j) {
                    assert(M(i,j) >= 0);
                    n += M(i,j);
                }
            }
            
            frequency_matrix_type F = M / n;
            return F;
        }
        
        /*! Pearson's chi-squared test tests the null hypothesis that the frequency
         distribution of observed events O is consistent with the frequency
         distribution of expected events E.
         
         \warning This test requires that all entries in E are positive.
         */
        template <typename FrequenceMatrix>
        double pearson_chi_squared(const FrequenceMatrix& O, const FrequenceMatrix& E) {
            assert(O.size1() == E.size1());
            assert(O.size2() == E.size2());
            double x2=0.0;
            for(std::size_t i=0; i<O.size1(); ++i) {
                for(std::size_t j=0; j<O.size2(); ++j) {
                    x2 += pow(O(i,j)-E(i,j), 2.0) / E(i,j);
                }
            }
            return x2;
        }
        
        /*! Calculates Cramer's V, a measure of association between O (observed) and
         E (expected) frequency distributions given N samples.
         
         It has a range of 0 (no association) to 1 (perfect association).
         */
        template <typename FrequenceMatrix>
        double cramers_v(const FrequenceMatrix& O, const FrequenceMatrix& E, std::size_t N) {
            assert(O.size1() == E.size1());
            assert(O.size2() == E.size2());
            double k=static_cast<double>(std::min(O.size1(), O.size2()));
            return sqrt(pearson_chi_squared(O,E)/(static_cast<double>(N)*(k-1.0)));
        }
        
        /*! This calculates the sum squared error between the frequency distributions
         of X (actual) and Y (predicted) variables.  X and Y may hold up to n possible
         values, and we assume that these values are in the half-open range [0,n).
         */
        template <typename Sequence>
        double sum_squared_error(const Sequence& X, const Sequence& Y, std::size_t n) {
            assert(X.size() > 0);
            assert(X.size() == Y.size());
            confusion_matrix_type Cpredicted = confusion_matrix(X,Y,n);
            confusion_matrix_type Cactual = confusion_matrix(X,X,n);
            
            double N=static_cast<double>(X.size());
            frequency_matrix_type O = Cpredicted / N;
            frequency_matrix_type E = Cactual / N;
            frequency_matrix_type Err = O - E;
            Err = boost::numeric::ublas::element_prod(Err,Err);
            
            double sse=0.0;
            for(std::size_t i=0; i<Err.size1(); ++i) {
                for(std::size_t j=0; j<Err.size2(); ++j) {
                    sse += Err(i,j);
                }
            }

            return sse;
        }
        
        /*! This calculates the sum absolute error between the frequency distributions
         of X (actual) and Y (predicted) variables.  X and Y may hold up to n possible
         values, and we assume that these values are in the half-open range [0,n).
         */
        template <typename Sequence>
        double sum_abs_error(const Sequence& X, const Sequence& Y, std::size_t n) {
            assert(X.size() > 0);
            assert(X.size() == Y.size());
            confusion_matrix_type Cpredicted = confusion_matrix(X,Y,n);
            confusion_matrix_type Cactual = confusion_matrix(X,X,n);
            
            double N=static_cast<double>(X.size());
            frequency_matrix_type O = Cpredicted / N;
            frequency_matrix_type E = Cactual / N;
            frequency_matrix_type Err = O - E;
            
            double sae=0.0;
            for(std::size_t i=0; i<Err.size1(); ++i) {
                for(std::size_t j=0; j<Err.size2(); ++j) {
                    sae += fabs(Err(i,j));
                }
            }
            
            return sae;
        }
        
    } // analysis
} // ea

#endif
