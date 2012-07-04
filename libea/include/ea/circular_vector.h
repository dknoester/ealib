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
#ifndef _EA_CIRCULAR_VECTOR_H_
#define _EA_CIRCULAR_VECTOR_H_

#include <vector>

namespace ea {

	template <typename ForwardIterator>
    class circular_iterator {
        
        // begin=vector begin
        // end = vector begin + loop counter 1
        // advance to end ++, increment loop counter
    };
    
    
	template <typename T>
	class circular_vector : public std::vector<T> {
	public:
        typedef circular_vector<T> this_type;
		typedef std::vector<T> base_type;
		typedef typename base_type::reference reference;
		typedef typename base_type::const_reference const_reference;
		
		circular_vector() : base_type() {
		}

        this_type& operator=(const this_type& that) {
            if(this != &that) {
                base_type::operator=(that);
            }
            return *this;
        }

		template <typename ForwardIterator>
		circular_vector(ForwardIterator f, ForwardIterator l) : base_type(f,l) {
		}
		
		inline reference operator[](std::size_t i) {
			return base_type::operator[](i % base_type::size());
		}	
		
		inline const_reference operator[](std::size_t i) const {
			return base_type::operator[](i % base_type::size());
		}
        
        inline std::size_t size() const {
            return base_type::size();
        }
	};
	
} // ea


#endif
