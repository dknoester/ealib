/* circular_vector.h
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
#ifndef _EA_CVECTOR_H_
#define _EA_CVECTOR_H_

#include <boost/iterator/iterator_adaptor.hpp>
#include <vector>

namespace ea {

    /*! Iterator class for cvector.
     
     The trick with circular iterators is that you never actually reach the end -
     you always loop around to the beginning.  So, in order to check i!=end(),
     we use a loop counter.
     */
	template <typename ForwardIterator>
    class citerator 
    : public 
    boost::iterator_adaptor<
    citerator<ForwardIterator>, // derived
    ForwardIterator, // base
    boost::use_default,
    boost::forward_traversal_tag> {
    public:
        typedef citerator<ForwardIterator> this_type;
        
        //! Constructs a citerator from a loop counter and set of begin, end, and current iterators.
        citerator(unsigned int lc, ForwardIterator f, ForwardIterator l, ForwardIterator c) : _loop_count(lc), _begin(f), _end(l), _cur(c) {
        }
        
        //! Constructs a citerator from another citerator with a different current position.
        citerator(const citerator& that, ForwardIterator c) {
            *this = that;
            _cur = c;
        }
        
        //! Assignment operator.
        citerator& operator=(const citerator& that) {
            if(this != &that) {
                _loop_count = that._loop_count;
                _begin = that._begin;
                _end = that._end;
                _cur = that._cur;
            }
            return *this;
        }

    protected:
        template <typename> friend class cvector;
        unsigned int _loop_count; //!< Loop counter.
        ForwardIterator _begin, _end, _cur; //!< Range and current iterators.

    private:
        friend class boost::iterator_core_access;
        
        //! Advance this iterator.
        void increment() {
            if(++_cur == _end) {
                _cur = _begin;
                ++_loop_count;
            }
        }

        //! Compare iterators for equality.
        bool equal(const this_type& that) const {
            return ((_begin == that._begin)
                    && (_end == that._end)
                    && (_cur == that._cur)
                    && (_loop_count == that._loop_count));
        }

        //! Dereference this iterator.
        typename this_type::iterator_adaptor_::reference dereference() const {
            return *_cur;
        }
        
        //! Advance this iterator by n.
        void advance(typename this_type::iterator_adaptor_::difference_type n) {
            // count complete loops:
            typename this_type::iterator_adaptor_::difference_type d=std::distance(_begin,_end);
            _loop_count += n / d;
            n %= d;
            
            // now, handle wrap-around:
            d = std::distance(_cur,_end);
            if(n < d) {
                _cur += n;
            } else {
                ++_loop_count;
                _cur = _begin + (n-d);
            }
        }
    };
        
    
    /*! Circular vector.
     
     The key difference between cvector and std::vector is that advancing an iterator
     never "falls off" the end of the vector.  Once an iterator is advanced to end(),
     it loops to begin().  Checking for i!=end() is still valid via a loop counter
     in the iterator.
     */
	template <typename T>
	class cvector : public std::vector<T> {
	public:
        typedef cvector<T> this_type;
        typedef std::vector<T> base_type;
        typedef typename base_type::value_type value_type;
        typedef typename base_type::pointer pointer;        
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
        typedef typename base_type::size_type size_type;
        typedef typename base_type::difference_type difference_type;
        typedef citerator<typename base_type::iterator> iterator;
        typedef citerator<typename base_type::const_iterator> const_iterator;
        typedef citerator<typename base_type::reverse_iterator> reverse_iterator;
        typedef citerator<typename base_type::const_reverse_iterator> const_reverse_iterator;

		//! Constructs an empty cvector.
		cvector() : base_type() {
		}

        //! Constructs a cvector with n elements.
        cvector(size_type n)	: base_type(n) {
        }

        //! Constructs a cvector with n copies of t.
        cvector(size_type n, const T& t) : base_type(n,t) {
        }

        //! Constructs a cvector with a copy of the range [f,l).
		template <typename ForwardIterator>
		cvector(ForwardIterator f, ForwardIterator l) : base_type(f,l) {
		}
        
        //! Copy constructor.
        cvector(const cvector& that) : base_type(that) {
        }
        
        //! Destructor.
        virtual ~cvector() {
        }

        //! Assignment operator.
        this_type& operator=(const this_type& that) {
            if(this != &that) {
                base_type::operator=(that);
            }
            return *this;
        }

		//! Returns a reference to the i'th element.
		inline reference operator[](std::size_t i) {
			return base_type::operator[](i % base_type::size());
		}	
		
        //! Returns a const reference to the i'th element.
		inline const_reference operator[](std::size_t i) const {
			return base_type::operator[](i % base_type::size());
		}
        
        //! Returns an iterator to the beginning of the cvector.
        iterator begin() {
            return iterator(0, base_type::begin(), base_type::end(), base_type::begin());
        }

        //! Returns an iterator to the end of the cvector.
        iterator end() {
            return iterator(1, base_type::begin(), base_type::end(), base_type::begin());
        }
        
        //! Returns an iterator to the beginning of the cvector (const-qualified).
        const_iterator begin() const {
            return const_iterator(0, base_type::begin(), base_type::end(), base_type::begin());
        }
        
        //! Returns an iterator to the end of the cvector (const-qualified).
        const_iterator end() const {
            return const_iterator(1, base_type::begin(), base_type::end(), base_type::begin());
        }

        //! Returns a reverse iterator to the beginning (end) of the cvector.
        reverse_iterator rbegin() {
            return reverse_iterator(0, base_type::rbegin(), base_type::rend(), base_type::rbegin());
        }
        
        //! Returns a reverse iterator to the end (beginning) of the cvector.
        reverse_iterator rend() {
            return reverse_iterator(1, base_type::rbegin(), base_type::ernd(), base_type::rbegin());
        }
        
        //! Returns a reverse iterator to the beginning (end) of the cvector (const-qualified).
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(0, base_type::rbegin(), base_type::rend(), base_type::rbegin());
        }
        
        //! Returns a reverse iterator to the end (beginning) of the cvector (const-qualified).
        const_reverse_iterator rend() const {
            return const_reverse_iterator(1, base_type::rbegin(), base_type::rend(), base_type::rbegin());
        }
        
        //! Inserts x before pos.
        iterator insert(iterator pos, const T& x) {
            typename base_type::iterator i=base_type::insert(pos._cur,x);
            return iterator(0, base_type::begin(), base_type::end(), i);
        }
        
        //! Inserts the range [first, last) before pos.
        template <class InputIterator>
        void insert(iterator pos, InputIterator f, InputIterator l) {
            base_type::insert(pos._cur, f, l);
        }

        //! Inserts n copies of x before pos.
        void insert(iterator pos, size_type n, const T& x) {
            base_type::insert(pos, n, x);
        }

        //! Erases the element at position pos.
        iterator erase(iterator pos) {
            typename base_type::iterator i=base_type::erase(pos._cur);
            return iterator(0, base_type::begin(), base_type::end(), i);
        }
        
        //! Erases the range [first, last).
        iterator erase(iterator f, iterator l) {
            typename base_type::iterator i=base_type::erase(f._cur,l._cur);
            return iterator(0, base_type::begin(), base_type::end(), i);
        }
	};
	
} // ea


#endif
