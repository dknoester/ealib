/* circular_genome.h
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
#ifndef _EA_GENOME_TYPES_CIRCULAR_GENOME_H_
#define _EA_GENOME_TYPES_CIRCULAR_GENOME_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <sstream>
#include <ea/metadata.h>
#include <ea/data_structures/circular_vector.h>


namespace ealib {
	
	/*! Canonical numeric representation for genetic algorithms.
	 */
	template <typename T>
	struct circular_genome : public cvector<T> {
		//! Type of this representation.
		typedef circular_genome<T> representation_type;
		//! Type of codon in this genome.
		typedef T codon_type;
		//! Base type of this representation.
		typedef cvector<T> base_type;
		
		//! Constructor.
		circular_genome() : base_type() {
		}
        
        //! Another constructor.
        template <typename InputIterator>
		circular_genome(InputIterator f, InputIterator l) : base_type(f, l) {
		}
        
        //! Assignment operator.
        representation_type& operator=(const representation_type& that) {
            if(this != &that) {
                base_type::operator=(that);
            }
            return *this;
        }
		
		//! Constructor that initializes to the given size.
		circular_genome(const std::size_t n) : base_type(n) {
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
