/* gates.h
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
#ifndef _EA_MKV_GATES_H_
#define _EA_MKV_GATES_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <vector>
#include <deque>

#include <ea/algorithm.h>

namespace ealib {
    namespace mkv {
        
        namespace bnu = boost::numeric::ublas;
        
        typedef bnu::vector<int> state_vector_type; //!< State vector type.
        typedef bnu::vector<std::size_t> index_vector_type; //!< Index vector (logical vector).
        typedef bnu::vector<double> weight_vector_type; //!< Type for feedback weights vector.
        
        typedef bnu::matrix<double> matrix_type; //!< Probability table type.
        typedef bnu::matrix_column<matrix_type> column_type; //!< Column type.
        typedef bnu::matrix_row<matrix_type> row_type; //!< Row type.
        
        /*! Abstract gate.
         */
        template <typename RandomNumberGenerator>
        struct abstract_gate {
            //! Default constructor.
            abstract_gate() {
            }
            
            //! Destructor.
            virtual ~abstract_gate() {
            }
            
            //! Return the output of this gate based on input x.
            virtual int operator()(int x, RandomNumberGenerator& rng) = 0;
            
            index_vector_type inputs; //!< Input indices to this node.
            index_vector_type outputs; //!< Output indices from this node.
        };
        
        
        /*! Logic gate.
         */
        template <typename RandomNumberGenerator>
        struct logic_gate : abstract_gate<RandomNumberGenerator> {
            typedef abstract_gate<RandomNumberGenerator> base_type;
            
            //! Constructor.
            logic_gate() {
            }
            
            //! Destructor.
            virtual ~logic_gate() {
            }
            
            //! Return the output of this gate based on input x.
            virtual int operator()(int x, RandomNumberGenerator& rng) {
                return M[x];
            }
            
            index_vector_type M; //!< Truth table.
        };
        
        
        /*! Probabilistic (Markov) gate.
         */
        template <typename RandomNumberGenerator>
        struct probabilistic_gate : abstract_gate<RandomNumberGenerator> {
            typedef abstract_gate<RandomNumberGenerator> base_type;
            
            //! Constructor.
            probabilistic_gate() {
            }
            
            //! Destructor.
            virtual ~probabilistic_gate() {
            }
            
            //! Update t+1 with input from t.
            virtual int operator()(int x, RandomNumberGenerator& rng) {
                row_type row(M, x);
                double p = rng.p();
                
                for(int j=0; j<static_cast<int>(M.size2()); ++j) {
                    if(p <= row[j]) {
                        return j;
                    }
                    p -= row[j];
                }
                
                // if we get here, there was a floating point precision problem.
                // default to the final column:
                return M.size2()-1;
            }
            
            //! Convenience method for normalizing the probability table.
            void normalize() {
                for(std::size_t i=0; i<M.size1(); ++i) {
                    row_type row(M, i);
                    algorithm::normalize(row.begin(), row.end(), 1.0);
                }
            }
            
            matrix_type M; //!< Probability table.
        };
        
        
        /*! Adaptive Markov gate.
         */
        template <typename RandomNumberGenerator>
        struct adaptive_gate : abstract_gate<RandomNumberGenerator> {
            typedef abstract_gate<RandomNumberGenerator> base_type;
            typedef std::deque<std::pair<std::size_t, std::size_t> > history_type;//!< Type for tracking history of decisions.
            
            //! Constructor.
            adaptive_gate() {
            }
            
            //! Destructor.
            virtual ~adaptive_gate() {
            }
            
            //! Update t+1 with input from t.
            virtual int operator()(int x, RandomNumberGenerator& rng) {
                // learn first: if one of the feedback bits is on, it means that
                // the previous behavior of this gate should be reinforced.
                // if we wait to learn until after updating, we'll be reinforcing
                // for the *next* output as well.
                while(H.size() > h) { // prune history
                    H.pop_front();
                }
                
                if(x & 0x01) { // reinforce
                    reinforce();
                }
                if((x>>1) & 0x01) { // inhibit
                    inhibit();
                }
                x = x >> 2; // lop off the two feedback bits
                
                // now handle the next output:
                row_type row(M, x);
                double p = rng.p();
                
                for(int j=0; j<static_cast<int>(M.size2()); ++j) {
                    if(p <= row[j]) {
                        H.push_back(std::make_pair(x,j));
                        return j;
                    }
                    p -= row[j];
                }
                
                // if we get here, there was a floating point precision problem.
                // default to the final column:
                H.push_back(std::make_pair(x,static_cast<int>(M.size2()-1)));
                return M.size2()-1;
            }
            
            //! Convenience method for normalizing the probability table.
            void normalize() {
                for(std::size_t i=0; i<M.size1(); ++i) {
                    row_type row(M, i);
                    algorithm::normalize(row.begin(), row.end(), 1.0);
                }
            }
            
            //! Scale the probability of output (i,j) by s.
            void scale(std::size_t i, std::size_t j, double s) {
                M(i,j) *= 1.0 + s;
                row_type row(M, i);
                algorithm::normalize(row.begin(), row.end(), row.begin(), 1.0);
            }
            
            //! Reinforce the recent behavior of this gate.
            void reinforce() {
                for(std::size_t i=0; (i<P.size()) && (i<H.size()); ++i) {
                    scale(H[i].first, H[i].second, P[i]);
                }
            }
            
            //! Inhibit the recent behavior of this gate.
            void inhibit() {
                for(std::size_t i=0; (i<N.size()) && (i<H.size()); ++i) {
                    scale(H[i].first, H[i].second, N[i]);
                }
            }
            
            std::size_t h; //!< Size of history to keep.
            history_type H; //!< History of decisions made by this node.
            weight_vector_type P; //!< Positive feedback weight vector.
            weight_vector_type N; //!< Negative feedback weight vector.
            matrix_type M; //!< Probability table.
        };
        
    } // mkv
} // ealib

#endif
