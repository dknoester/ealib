/* torus.h
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
#ifndef _EA_TORUS_H_
#define _EA_TORUS_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <vector>

namespace ealib {
    
    /*! N-dimensional toroidal container.
     */
    template <typename T, int Dimension=1>
    class torus {
    public:
        typedef T value_type;
        typedef value_type& reference;
        typedef const reference const_reference;
        typedef boost::numeric::ublas::matrix<value_type> storage_type;
        typedef typename storage_type::array_type::iterator iterator;
        
        torus(std::size_t m, std::size_t n, const T& t=T()) : _M(boost::numeric::ublas::scalar_matrix<value_type>(m,n,t)) {
        }
        
        template <typename ForwardIterator>
        void fill(ForwardIterator f, ForwardIterator l) {
            for(std::size_t i=0; i<_M.size1(); ++i) {
                for(std::size_t j=0; j<_M.size2(); ++j) {
                    if(f == l) {
                        return;
                    }
                    _M(i,j) = *f++;
                }
            }
        }
        
        reference operator()(int i, int j) {
            return _M(rebase(i,_M.size1()), rebase(j,_M.size2()));
        }
        
        const_reference operator()(int i, int j) const {
            return _M(rebase(i,_M.size1()), rebase(j,_M.size2()));
        }
        
        iterator begin() { return _M.data().begin(); }
        iterator end() { return _M.data().end(); }
        
        std::size_t size() const { return _M.size1() * _M.size2(); }
        std::size_t size1() const { return _M.size1(); }
        std::size_t size2() const { return _M.size2(); }
        
    protected:
        //! Rebase index x to size y.
        inline std::size_t rebase(int x, std::size_t y) {
            if(x >= 0) {
                return x % y;
            } else {
                x = (-1*x) % y;
                return y - x;
            }
        }
        
        storage_type _M; //!< Underlying storage for the data held in each cell of the torus.
    };
    
    
    template <typename T>
    struct torus_offset {
        typedef typename T::reference reference;
        
        torus_offset(T& t, int i, int j) : _t(t), _i(i), _j(j) {
        }
        
        reference operator()(int i, int j) {
            return _t(i+_i, j+_j);
        }
        
        T& _t;
        int _i, _j;
    };
    
    
    template <typename T>
    struct adaptor_2d {
        adaptor_2d(T& t, std::size_t m, size_t n) : _t(t), _m(m), _n(n) {
        }
        
        typename T::reference operator[](std::size_t i) {
            return _t(i/_n, i%_n);
        }
        
        T& _t;
        std::size_t _m, _n;
    };
    
} // ea

#endif
