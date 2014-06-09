/* torus2.h
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
#ifndef _EA_TORUS2_H_
#define _EA_TORUS2_H_

#include <boost/iterator/iterator_adaptor.hpp>
#include <vector>

namespace ealib {
    /*
     
     */
    template
    < typename T // T is a pointer-to-individual type; it should be autoptr-like, e.g., boost::shared_ptr
    template <typename> class Population=shared_ptr_vector
    template <typename> class Topology=torus2
    > class environment {
    public:
        typedef Population<T> population_type;
        typedef typename population_type::iterator population_iterator;
        typedef Topology<T> topology_type;
        typedef topology_type::location_type location_type;
        typedef typename topology_type::iterator topology_iterator;
        typedef typename topology_type::neighborhood_iterator neighborhood_iterator;
        
        //! Constructor.
        environment() {
        }
        
        population_iterator begin_population() {
            return _population.begin();
        }
        
        population_iterator end_population() {
            return _population.end();
        }
        
        topology_iterator begin_topology() {
            return _topology.begin();
        }
        
        topology_iterator end_topology() {
            return _topology.end();
        }
        
        neighborhood_iterator begin_neighborhood(const location_type& l) {
            return _topology.begin_neighborhood(l);
        }
        
        neighborhood_iterator end_neighborhood(const location_type& l) {
            return _topology.end_neighborhood(l);
        }
        
        // insert into the population; location is first avail, throws if full
        insert(population_iterator i, T& t) {
            _population.insert(i,t);
            _topology.insert(t);
        }
        
        // insert into a location; appended to population
        insert(topology_iterator i, T& t) {
            _population.insert(_population.end(), t);
            _topology.insert(i,t);
        }
        
    private:
        population_type _population; //!< Underlying storage for the population.
        topology_type _topology; //!< Underlying storage for the topology.
    };

    /*! 2-dimensional toroidal container.
     */
    template
    < typename T
    , typename StorageType=std::vector<T>
    > class torus2 {
    public:
        typedef StorageType storage_type;
        typedef typename storage_type::value_type value_type;
        typedef typename storage_type::reference reference;
        typedef typename storage_type::pointer pointer;
        typedef typename storage_type::difference_type difference_type;
        typedef typename storage_type::size_type size_type;
        typedef typename 
        typedef typename int index_type;
 
        //! Constructor.
        torus2(size_type x, size_type y) : _sizex(x), _sizey(y), _storage(x*y) {
        }
        
        //! Returns a reference to the element at position (x,y).
        reference operator()(index_type x, index_type y) {
            return data()[rebase(y,_sizey)*_sizex + rebase(x,_sizex)];
        }
        
        //! Returns a reference to the element at position (x,y) (const-qualified).
        const_reference operator()(int i, int j) const {
            return data()[rebase(y,_sizey)*_sizex + rebase(x,_sizex)];
        }
        
        //! Returns a reference to the underlying storage type.
        storage_type& data() { return _storage; }

        //! Returns a reference to the underlying storage type (const-qualified).
        const storage_type& data() const { return _storage; }
        
        //! Returns the total size of this torus.
        size_type size() const { return _storage.size() }
        
        //! Returns the size of the x-dimension of this torus.
        size_type sizex() const { return _sizex; }
        
        //! Returns the size of the y-dimension of this torus.
        size_type sizey() const { return _sizey; }

    protected:
        //! Rebase index i to size j.
        inline size_type rebase(int i, size_type j) {
            if(i >= 0) {
                return i % j;
            } else {
                i = (-1*i) % j;
                return j - i;
            }
        }
        
        storage_type _storage; //!< Underlying storage for elements in the torus.
        size_type _sizex, _sizey; //!< Size in x and y dimensions.
        

        
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
        typedef typename T::value_type value_type;
        
        offset_torus2(T* t=0, int i=0, int j=0) : _t(t), _i(i), _j(j) {
        }
        
        inline void reset(int i, int j) {
            _i = i; _j = j;
        }
        
        inline void reset(T* t) {
            _t = t;
        }
        
        reference operator()(int i, int j) {
            return (*_t)(i+_i, j+_j);
        }
        
        T* _t;
        int _i, _j;
    };
    
    
    template <typename T>
    struct adaptor_torus2 {
        typedef typename T::reference reference;
        typedef typename T::value_type value_type;
        
        adaptor_torus2(T& t, std::size_t m, size_t n) : _t(t), _m(m), _n(n) {
        }
        
        reference operator[](std::size_t i) {
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
        typedef typename T::value_type value_type;
        
        offset_torus3(T* t=0, int i=0, int j=0, int k=0) : _t(t), _i(i), _j(j), _k(k) {
        }
        
        inline void reset(int i, int j, int k) {
            _i = i; _j = j; _k = k;
        }
        
        inline void reset(T* t) {
            _t = t;
        }
        
        reference operator()(int i, int j, int k) {
            return (*_t)(i+_i, j+_j, k+_k);
        }
        
        T* _t;
        int _i, _j, _k;
    };
    
    
    template <typename T>
    struct adaptor_torus3 {
        typedef typename T::reference reference;
        typedef typename T::value_type value_type;
        
        adaptor_torus3(T& t, std::size_t m, size_t n, std::size_t p) : _t(t), _m(m), _n(n), _p(p) {
        }
        
        reference operator[](std::size_t i) {
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
