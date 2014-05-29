/* numeric_vector.h
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
#ifndef _EA_GENOME_TYPES_NUMERIC_VECTOR_H_
#define _EA_GENOME_TYPES_NUMERIC_VECTOR_H_

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <sstream>

#include <vector>
#include <ea/meta_data.h>


namespace ealib {
	
	/*! Canonical numeric representation for genetic algorithms.
	 */
	template <typename T>
	struct numeric_vector : public std::vector<T> {
		//! Type of this representation.
		typedef numeric_vector<T> representation_type;
		//! Base type of this representation.
		typedef std::vector<T> base_type;
        //! Type of codon used in this genome.
        typedef T codon_type;
		
		//! Constructor.
		numeric_vector() : base_type() { }

		//! Copy constructor.
        numeric_vector(const numeric_vector& that) : base_type(that) {
        }
        
		//! Constructor that initializes to the given size.
		numeric_vector(const std::size_t n) : base_type(n) { }

        //! Constructor that initializes with the range [f,l).
        template <typename InputIterator>
		numeric_vector(InputIterator f, InputIterator l) : base_type(f, l) {
		}

        //! Assignment operator.
        numeric_vector& operator=(const numeric_vector& that) {
            if(this != &that) {
                static_cast<base_type*>(this)->operator=(that);
            }
            return *this;
        }

        // These enable a more compact serialization of the genome.
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
			std::ostringstream out;
			out << base_type::size();
			for(typename base_type::const_iterator i=base_type::begin(); i!=base_type::end(); ++i) {
				out << " " << *i;
			}
			std::string genome(out.str());
			ar & BOOST_SERIALIZATION_NVP(genome);
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
			std::string genome;
			ar & BOOST_SERIALIZATION_NVP(genome);
			std::istringstream in(genome);
			std::size_t s;
			in >> s;
			base_type::reserve(s);
			T t;
			for( ; s>0; --s) {
				in >> t;
                base_type::push_back(t);
			}
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();		
	};
	
} // ea

#endif
