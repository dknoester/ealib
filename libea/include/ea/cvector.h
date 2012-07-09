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
        
        citerator(unsigned int lc, ForwardIterator f, ForwardIterator l) : _loop_count(lc), _begin(f), _end(l), _cur(f) {
        }

    protected:
        unsigned int _loop_count;
        ForwardIterator _begin, _end, _cur;

    private:
        friend class boost::iterator_core_access;
        
        void increment() {
            if(++_cur == _end) {
                _cur = _begin;
                ++_loop_count;
            }
        }

        bool equal(const this_type& that) const {
            return ((_begin == that._begin)
                    && (_end == that._end)
                    && (_cur == that._cur)
                    && (_loop_count == that._loop_count));
        }

        typename this_type::iterator_adaptor_::reference dereference() const {
            return *_cur;
        }
        
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
        
    
	template <typename T>
	class cvector : public std::vector<T> {
	public:
        typedef cvector<T> this_type;
		typedef std::vector<T> base_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
        typedef citerator<typename base_type::iterator> iterator;
//        typedef const_citerator<typename base_type::const_iterator> const_iterator;
		
		cvector() : base_type() {
		}

        this_type& operator=(const this_type& that) {
            if(this != &that) {
                base_type::operator=(that);
            }
            return *this;
        }

		template <typename ForwardIterator>
		cvector(ForwardIterator f, ForwardIterator l) : base_type(f,l) {
		}
		
		inline reference operator[](std::size_t i) {
			return base_type::operator[](i % base_type::size());
		}	
		
		inline const_reference operator[](std::size_t i) const {
			return base_type::operator[](i % base_type::size());
		}
        
        iterator begin() {
            return iterator(0, base_type::begin(), base_type::end());
        }
        
        iterator end() {
            return iterator(1, base_type::begin(), base_type::end());
        }
	};
	
} // ea


#endif
