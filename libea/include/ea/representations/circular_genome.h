#ifndef _EA_REPRESENTATIONS_CIRCULAR_GENOME_H_
#define _EA_REPRESENTATIONS_CIRCULAR_GENOME_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <sstream>
#include <ea/meta_data.h>
#include <ea/circular_vector.h>


namespace ea {
	
	/*! Canonical numeric representation for genetic algorithms.
	 */
	template <typename T>
	struct circular_genome : public circular_vector<T> {
		//! Type of this representation.
		typedef circular_genome<T> representation_type;
		//! Type of codon in this genome.
		typedef T codon_type;
		//! Base type of this representation.
		typedef circular_vector<T> base_type;
		
		//! Constructor.
		circular_genome() : base_type() {
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
