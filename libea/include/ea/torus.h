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

#include <ea/cvector.h>

namespace ealib {
    
    template <typename Container>
    class torus1_proxy {
    public:
        typedef Container container_type; //!< Type of embedded container.
        typedef typename container_type::value_type value_type;
        typedef typename container_type::pointer pointer;
		typedef typename container_type::reference reference;
		typedef typename container_type::const_reference const_reference;
        typedef typename container_type::size_type size_type;
        typedef typename container_type::difference_type difference_type;
        typedef circular_iterator<torus1_proxy,typename container_type::iterator> iterator;
        typedef circular_iterator<torus1_proxy,typename container_type::const_iterator> const_iterator;
        typedef circular_iterator<torus1_proxy,typename container_type::reverse_iterator> reverse_iterator;
        typedef circular_iterator<torus1_proxy,typename container_type::const_reverse_iterator> const_reverse_iterator;
        
        //! Constructor.
        torus1_proxy(Container& c) : _c(c) {
        }
        
        //!< Returns the size of the proxied container.
        size_type size() const { return _c.size(); }
        
        //! Returns a reference to element i.
		inline reference operator[](int i) {
            if(i >= 0) {
                return _c[i % _c.size()];
            } else {
                i = (-1*i) % _c.size();
                return _c[_c.size() - i];
            }
		}
		
        //! Returns a const reference to the i'th element.
		inline const_reference operator[](int i) const {
            if(i >= 0) {
                return _c[i % _c.size()];
            } else {
                i = (-1*i) % _c.size();
                return _c[_c.size() - i];
            }
		}

        //! Returns an iterator to the beginning of the cvector.
        iterator begin() { return iterator(0, _c.begin(), _c.end(), _c.begin()); }
        
        //! Returns an iterator to the end of the cvector.
        iterator end() {
            return iterator(1, _c.begin(), _c.end(), _c.begin());
        }
        
        //! Returns an iterator to the beginning of the cvector (const-qualified).
        const_iterator begin() const {
            return const_iterator(0, _c.begin(), _c.end(), _c.begin());
        }
        
        //! Returns an iterator to the end of the cvector (const-qualified).
        const_iterator end() const {
            return const_iterator(1, _c.begin(), _c.end(), _c.begin());
        }
        
        //! Returns a reverse iterator to the beginning (end) of the cvector.
        reverse_iterator rbegin() {
            return reverse_iterator(0, _c.rbegin(), _c.rend(), _c.rbegin());
        }
        
        //! Returns a reverse iterator to the end (beginning) of the cvector.
        reverse_iterator rend() {
            return reverse_iterator(1, _c.rbegin(), _c.ernd(), _c.rbegin());
        }
        
        //! Returns a reverse iterator to the beginning (end) of the cvector (const-qualified).
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(0, _c.rbegin(), _c.rend(), _c.rbegin());
        }
        
        //! Returns a reverse iterator to the end (beginning) of the cvector (const-qualified).
        const_reverse_iterator rend() const {
            return const_reverse_iterator(1, _c.rbegin(), _c.ernd(), _c.rbegin());
        }

        //! Erases the element at position pos.
        iterator erase(iterator pos) {
            typename container_type::iterator i=_c.erase(pos.current());
            return iterator(0, _c.begin(), _c.end(), i);
        }
        
        //! Erases the range [first, last).
        iterator erase(iterator f, iterator l) {
            // logically, we want to keep [begin,f) and [l,end).
            // however, f and l can be any place in the sequence.
            container_type t;
            bool skip = false;
            for(typename container_type::iterator i=_c.begin(); i!=_c.end(); ++i) {
                // found f; start skipping elements.
                if(i==f.current()) {
                    skip = true;
                }
                // found l; we're done skipping, but we might have copied too much
                if(i==l.current()) {
                    if(!skip) {
                        // l occured before f; erase what we have so far
                        t.clear();
                    }
                    skip = false; // we always stop skipping once we cross l.
                }
                if(!skip) {
                    t.insert(t.end(), *i);
                }
            }
            std::swap(_c, t);
            
            return begin();
        }

    protected:
        torus1_proxy(); // no default constructor.
        
        container_type _c; //!< Proxied container.
    };
    
    
    /*! 2-dimensional toroidal container.
     */
    template <typename T>
    class torus2 {
    public:
        typedef T value_type;
        typedef value_type& reference;
        typedef const reference const_reference;
        typedef boost::numeric::ublas::matrix<value_type> storage_type;
        typedef typename storage_type::array_type::iterator iterator;
        
        torus2(std::size_t m, std::size_t n, const T& t=T()) : _M(boost::numeric::ublas::scalar_matrix<value_type>(m,n,t)) {
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
    struct offset_torus2 {
        typedef typename T::reference reference;
        
        offset_torus2(T& t, int i, int j) : _t(t), _i(i), _j(j) {
        }
        
        reference operator()(int i, int j) {
            return _t(i+_i, j+_j);
        }
        
        T& _t;
        int _i, _j;
    };
    
    
    template <typename T>
    struct adaptor_torus2 {
        adaptor_torus2(T& t, std::size_t m, size_t n) : _t(t), _m(m), _n(n) {
        }
        
        typename T::reference operator[](std::size_t i) {
            return _t(i/_n, i%_n);
        }
        
        T& _t;
        std::size_t _m, _n;
    };
    
    
    /*! 3-dimensional toroidal container.
     */
    template <typename T>
    class torus3 {
    public:
        typedef std::vector<T> storage_type;
        typedef typename storage_type::iterator iterator;
        typedef typename storage_type::value_type value_type;
        typedef typename storage_type::reference reference;
        typedef typename storage_type::const_reference const_reference;
        
        torus3(std::size_t m, std::size_t n, std::size_t p, const T& t=T())
        : _m(m), _n(n), _p(p), _M(m*n*p, t) {
        }
        
        template <typename ForwardIterator>
        void fill(ForwardIterator f, ForwardIterator l) {
            for(std::size_t i=0; i<_M.size(); ++i) {
                if(f == l) {
                    return;
                }
                _M[i] = *f++;
            }
        }
        
        //! Returns the (i,j,k)'th element in this torus.
        reference operator()(int i, int j, int k) {
            std::size_t m=rebase(i,_m);
            std::size_t n=rebase(j,_n);
            std::size_t p=rebase(k,_p);
            return _M[_m*_n*p + _n*m + n];
        }

        //! Returns the (i,j,k)'th element in this torus (const-qualified).
        const_reference operator()(int i, int j, int k) const {
            std::size_t m=rebase(i,_m);
            std::size_t n=rebase(j,_n);
            std::size_t p=rebase(k,_p);
            return _M[_m*_n*p + _n*m + n];
        }

        iterator begin() { return _M.begin(); }
        iterator end() { return _M.end(); }
        
        std::size_t size() const { return _M.size(); }
        std::size_t size1() const { return _m; }
        std::size_t size2() const { return _n; }
        std::size_t size3() const { return _p; }
        
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
        
        std::size_t _m; //!< Logical number of rows.
        std::size_t _n; //!< Logical number of columns.
        std::size_t _p; //!< Logical number of pages.
        storage_type _M; //!< Underlying storage for the data held in each cell of the torus.
    };
    
    
    template <typename T>
    struct offset_torus3 {
        typedef typename T::reference reference;
        
        offset_torus3(T& t, int i, int j, int k) : _t(t), _i(i), _j(j), _k(k) {
        }
        
        reference operator()(int i, int j, int k) {
            return _t(i+_i, j+_j, k+_k);
        }
        
        T& _t;
        int _i, _j, _k;
    };
    
    
    template <typename T>
    struct adaptor_torus3 {
        adaptor_torus3(T& t, std::size_t m, size_t n, std::size_t p) : _t(t), _m(m), _n(n), _p(p) {
        }
        
        typename T::reference operator[](std::size_t i) {
            int p = i / (_m*_n);
            int r = i % (_m*_n);
            int m = r / _n;
            int n = r % _n;
            return _t(m,n,p);
        }
        
        T& _t;
        std::size_t _m, _n, _p;
    };
    
} // ea

#endif
