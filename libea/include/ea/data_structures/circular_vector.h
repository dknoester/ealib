/* circular_vector.h
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
#ifndef _EA_CIRCULAR_VECTOR_H_
#define _EA_CIRCULAR_VECTOR_H_

#include <boost/iterator/iterator_adaptor.hpp>
#include <vector>

namespace ealib {

    /*! Circular iterator class.
     
     The trick with circular iterators is that you never actually reach the end -
     you always loop around to the beginning.  So, in order to check i!=end(),
     we use a loop counter.
     */
	template <typename Container, typename ForwardIterator>
    class circular_iterator 
    : public boost::iterator_adaptor
    < circular_iterator<Container,ForwardIterator> // derived
    , ForwardIterator // base
    , boost::use_default
    , boost::forward_traversal_tag
    > {
    public:
        typedef Container container_type; // Type of container that we are circularly-iterating over.
        
        //! Constructs a circular_iterator from a loop counter and set of begin, end, and current iterators.
        circular_iterator(unsigned int lc, ForwardIterator f, ForwardIterator l, ForwardIterator c) : _loop_count(lc), _begin(f), _end(l), _cur(c) {
        }
        
        //! Copy constructor.
        circular_iterator(const circular_iterator& that) {
            *this = that;
        }

		//! Constructs a circular_iterator from another circular_iterator with a different current position.
		circular_iterator(const circular_iterator& that, ForwardIterator c) {
			*this = that;
			_cur = c;
		}
		
        //! Assignment operator.
        circular_iterator& operator=(const circular_iterator& that) {
            if(this != &that) {
                _loop_count = that._loop_count;
                _begin = that._begin;
                _end = that._end;
                _cur = that._cur;
            }
            return *this;
        }
		
		//! Increment and assign (advance).
		circular_iterator& operator+=(typename circular_iterator::iterator_adaptor_::difference_type n) {
			std::advance(*this, n);
			return *this;
		}
		
		//! Increment (advance).
		circular_iterator operator+(typename circular_iterator::iterator_adaptor_::difference_type n) {
			circular_iterator i(*this);
			std::advance(i, n);
			return i;
		}
		
        ForwardIterator current() { return _cur; }
		
    protected:
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
        bool equal(const circular_iterator& that) const {
            // if this iterator points to an empty sequence, ignore the loop count:
            if(std::distance(_begin,_end) == 0) {
                return (_begin == that._begin)
                && (_end == that._end);
            }
            
            return ((_begin == that._begin)
                    && (_end == that._end)
                    && (_cur == that._cur)
                    && (_loop_count == that._loop_count));
        }

        //! Dereference this iterator.
        typename circular_iterator::iterator_adaptor_::reference dereference() const {
            return *_cur;
        }
        
        //! Advance this iterator by n.
        void advance(typename circular_iterator::iterator_adaptor_::difference_type n) {
            // count complete loops:
            typename circular_iterator::iterator_adaptor_::difference_type d=std::distance(_begin,_end);
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
     
     The key difference between circular_vector and std::vector is that advancing an iterator
     never "falls off" the end of the vector.  Once an iterator is advanced to end(),
     it loops to begin().  Checking for i!=end() is still valid via a loop counter
     in the iterator.
     */
	template <typename T>
	class circular_vector : public std::vector<T> {
	public:
        typedef circular_vector<T> this_type;
        typedef std::vector<T> base_type;
        typedef typename base_type::value_type value_type;
        typedef typename base_type::pointer pointer;        
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
        typedef typename base_type::size_type size_type;
        typedef typename base_type::difference_type difference_type;
        typedef circular_iterator<circular_vector,typename base_type::iterator> iterator;
        typedef circular_iterator<circular_vector,typename base_type::const_iterator> const_iterator;
        typedef circular_iterator<circular_vector,typename base_type::reverse_iterator> reverse_iterator;
        typedef circular_iterator<circular_vector,typename base_type::const_reverse_iterator> const_reverse_iterator;

		//! Constructs an empty circular_vector.
		circular_vector() : base_type() {
		}

        //! Constructs a circular_vector with n elements.
        circular_vector(size_type n) : base_type(n) {
        }

        //! Constructs a circular_vector with n copies of t.
        circular_vector(size_type n, const T& t) : base_type(n,t) {
        }

        //! Constructs a circular_vector with a copy of the range [f,l).
		template <typename ForwardIterator>
		circular_vector(ForwardIterator f, ForwardIterator l) : base_type(f,l) {
		}
        
        //! Copy constructor.
        circular_vector(const circular_vector& that) : base_type(that) {
        }
        
        //! Destructor.
        virtual ~circular_vector() {
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

        //! Returns a reference to element i.
		inline reference operator[](int i) {
            if(i >= 0) {
                return base_type::operator[](i % base_type::size());
            } else {
                i = (-1*i) % base_type::size();
                return base_type::operator[](base_type::size()-i);
            }
		}
		
        //! Returns a const reference to the i'th element.
		inline const_reference operator[](int i) const {
            if(i >= 0) {
                return base_type::operator[](i % base_type::size());
            } else {
                i = (-1*i) % base_type::size();
                return base_type::operator[](base_type::size()-i);
            }
		}

        //! Returns an iterator to the beginning of the circular_vector.
        iterator begin() {
            return iterator(0, base_type::begin(), base_type::end(), base_type::begin());
        }

        //! Returns an iterator to the end of the circular_vector.
        iterator end() {
            return iterator(1, base_type::begin(), base_type::end(), base_type::begin());
        }
        
        //! Returns an iterator to the beginning of the circular_vector (const-qualified).
        const_iterator begin() const {
            return const_iterator(0, base_type::begin(), base_type::end(), base_type::begin());
        }
        
        //! Returns an iterator to the end of the circular_vector (const-qualified).
        const_iterator end() const {
            return const_iterator(1, base_type::begin(), base_type::end(), base_type::begin());
        }

        //! Returns a reverse iterator to the beginning (end) of the circular_vector.
        reverse_iterator rbegin() {
            return reverse_iterator(0, base_type::rbegin(), base_type::rend(), base_type::rbegin());
        }
        
        //! Returns a reverse iterator to the end (beginning) of the circular_vector.
        reverse_iterator rend() {
            return reverse_iterator(1, base_type::rbegin(), base_type::ernd(), base_type::rbegin());
        }
        
        //! Returns a reverse iterator to the beginning (end) of the circular_vector (const-qualified).
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(0, base_type::rbegin(), base_type::rend(), base_type::rbegin());
        }
        
        //! Returns a reverse iterator to the end (beginning) of the circular_vector (const-qualified).
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
            base_type::insert(pos.current(), f, l);
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
            typename base_type::iterator i=base_type::erase(f.current(),l.current());
            return iterator(0, base_type::begin(), base_type::end(), i);
        }
	};
    
    template <typename circular_vector>
    struct circular_vector_offset {
        typedef circular_vector circular_vector_type;
        typedef typename circular_vector::value_type value_type;
        
        circular_vector_offset(circular_vector_type* cv=0, int offset=0) : _cv(cv), _offset(offset) {
        }
        
        inline void reset(int offset) {
            _offset = offset;
        }
        
        inline void reset(circular_vector_type* cv) {
            _cv = cv;
        }
        
        inline typename circular_vector_type::reference operator[](int i) {
            return (*_cv)[i+_offset];
        }
        
        circular_vector_type* _cv; //!< Underlying circular_vector.
        int _offset; //!< Offset to apply to all indexing operations.
    };
	
} // ea


#endif
