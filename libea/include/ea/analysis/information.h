/* information.h 
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

#ifndef _EA_ANALYSIS_INFORMATION_H_
#define _EA_ANALYSIS_INFORMATION_H_

#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <ea/algorithm.h>

namespace ea {
    namespace analysis {

        //! Probability mass function.
        template <typename T>
        struct pmf {
            typedef std::vector<double> probability_table;
            typedef probability_table::iterator iterator;
            typedef std::map<T,unsigned int> events_type;
            
            pmf() : _n(0) {
            }
            
            void add(const T& t) {
                ++_e[t];
                ++_n;
            }
            
            void calc() {
                for(typename events_type::iterator i=_e.begin(); i!=_e.end(); ++i) {
                    t.push_back(i->second/static_cast<double>(_n));
                }
            }
            
            std::size_t event_count() const { return _n; }
            std::size_t size() const { return t.size(); }
            
            iterator begin() { return t.begin(); }
            iterator end() { return t.end(); }
            double& operator[](const std::size_t i) { return t[i]; }

            probability_table t;
            std::size_t _n;
            events_type _e;
        };
        
        
        //! Calculates the probability mass function of event sequence [f,l).
        template <typename ForwardIterator>
        pmf<typename ForwardIterator::value_type> probability_mass_function(ForwardIterator f, ForwardIterator l) {
            typedef typename ForwardIterator::value_type event_type;
            pmf<event_type> p;
            for( ; f!=l; ++f) {
                p.add(*f);
            }
            p.calc();
            return p;
        }
        
        
        //! Calculates the pmf for pointer types.
        template <class T>
        pmf<T> probability_mass_function(T* f, T* l) {
            pmf<T> p;
            for( ; f!=l; ++f) {
                p.add(*f);
            }
            p.calc();
            return p;
        }
        
        
        /*! Calculates the pmf for rows in a matrix.
         
         Although this could probably be avoided through a combination of boost::fusion and
         mpl, the corresponding tuple-based comparison simply uses a lexicographical ordering.
         So we do that here, too.
         */
        template <typename Matrix>
        pmf<std::string> probability_mass_function(Matrix& m) {
            typedef std::string event_type;
            pmf<event_type> p;
            for(unsigned i=0; i<m.size1(); ++i) {
                boost::numeric::ublas::matrix_row<Matrix> r(m,i);
                std::string s=ea::algorithm::vcat(r.begin(), r.end());
                p.add(s);
            }
            p.calc();
            return p;            
        }
        
        
        //! Calculates the entropy of the range [f,l).
        template <typename ForwardIterator>
        double entropy(ForwardIterator f, ForwardIterator l) {
            typedef typename ForwardIterator::value_type event_type;
            
            pmf<event_type> p=probability_mass_function(f,l);
            
            double s=0.0;
            for(typename pmf<event_type>::iterator i=p.begin(); i!=p.end(); ++i) {
                s += (*i) * log2(*i);
            }
            return -1.0 * s;
        }


        //! Calculates the entropy of event sequence [f,l), H(x).
        template <typename Sequence>
        double entropy(const Sequence& x) {
            return entropy(x.begin(), x.end());
        }
        
        
        //! Calculates the joint entropy of events in m; columns are variables, rows are events.
        template <typename Matrix>
        double joint_entropy(const Matrix& m) {
            typedef std::string event_type;
            
            pmf<std::string> p=probability_mass_function(m);
            
            double s=0.0;
            for(typename pmf<event_type>::iterator i=p.begin(); i!=p.end(); ++i) {
                s += (*i) * log2(*i);
            }
            return -1.0 * s;
        }
        

        //! Calculates conditional entropy H(X|Y).
        template <typename Sequence>
        double conditional_entropy(Sequence& x, Sequence& y) {
            using namespace boost::numeric::ublas;
            typedef typename Sequence::value_type value_type;
            typedef matrix<value_type> Matrix;
            typedef matrix_column<Matrix> Column;
            
            Matrix m(x.size(),2);
            Column(m,0) = x;
            Column(m,1) = y;
            return joint_entropy(m) - entropy(y);
        }

        
        //! Calculates the mutual information between event sequences x and y, I(x;y).
        template <typename Sequence>
        double mutual_information(Sequence& x, Sequence& y) {
            using namespace boost::numeric::ublas;
            typedef typename Sequence::value_type value_type;
            typedef matrix<value_type> Matrix;
            typedef matrix_column<Matrix> Column;

            Matrix m(x.size(),2);
            Column(m,0) = x;
            Column(m,1) = y;
            return entropy(x) + entropy(y) - joint_entropy(m);            
        }

        
        //! Calculates the conditional mutual information of event sequences x and y given z, I(x;y|z).
        template <typename Sequence>
        double conditional_mutual_information(Sequence& x, Sequence& y, Sequence& z) {
            using namespace boost::numeric::ublas;
            typedef typename Sequence::value_type value_type;
            typedef matrix<value_type> Matrix;
            typedef matrix_column<Matrix> Column;
            
            Matrix xz(x.size(),2);
            Column(xz,0) = x;
            Column(xz,1) = z;

            Matrix yz(y.size(),2);
            Column(yz,0) = y;
            Column(yz,1) = z;
            
            Matrix xyz(x.size(),3);
            Column(xyz,0) = x;
            Column(xyz,1) = y;
            Column(xyz,2) = z;
            
            return joint_entropy(xz) + joint_entropy(yz) - joint_entropy(xyz) - entropy(z);
        }


        //! Calculates the joint mutual information 
        template <typename Matrix, typename Sequence>
        double joint_mutual_information(Matrix& x, Sequence& y) {
            using namespace boost::numeric::ublas;
            typedef typename Sequence::value_type value_type;
            typedef matrix_column<Matrix> Column;
            typedef matrix_range<Matrix> Submatrix;
            
            Matrix xy(x.size1(), x.size2()+1);
            Submatrix(xy, range(0,xy.size1()), range(0,xy.size2()-1)) = x;
            Column(xy,xy.size2()-1) = y;

            return joint_entropy(x) + entropy(y) - joint_entropy(xy);
        }

    } // analysis
} // ea

#endif
