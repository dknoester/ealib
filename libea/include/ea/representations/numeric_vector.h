#ifndef _EA_REPRESENTATIONS_NUMERIC_VECTOR_H_
#define _EA_REPRESENTATIONS_NUMERIC_VECTOR_H_

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <sstream>

#include <vector>
#include <ea/meta_data.h>


namespace ea {
	
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
		
		//! Constructor that initializes to the given size.
		numeric_vector(const std::size_t n) : base_type(n) { }
		
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
	

    //! Bitstring representation type.
	typedef numeric_vector<int> bitstring;
    
    //! Integer-string representation type.
    typedef numeric_vector<int> intstring;
    
    //! Real-string representation type.
    typedef numeric_vector<double> realstring;
	
} // ea

#endif
