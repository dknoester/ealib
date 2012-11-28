/* population.h
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
#ifndef _EA_POPULATION_H_
#define _EA_POPULATION_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>

#include <ea/concepts.h>
#include <ea/meta_data.h>


namespace ea {
    
    /*! Population; a container for individuals.
	 */
	template <typename Individual, typename IndividualPtr>
	class population : public std::vector<IndividualPtr> {
	public:
        typedef Individual individual_type;
        typedef IndividualPtr individual_ptr_type;
		
		typedef std::vector<individual_ptr_type> base_type;
        typedef typename base_type::value_type value_type;
		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		
		//! Constructor.
		population() {
		}
        
        //! Initializing constructor.
        population(std::size_t n) : base_type(n) {
        }

        //! Initializing constructor.
        population(std::size_t n, const value_type& t) : base_type(n,t) {
        }

        //! Initializing constructor.
        template <typename ForwardIterator>
        population(ForwardIterator f, ForwardIterator l) : base_type(f,l) {
        }
        
        //! Destructor.
        virtual ~population() {
        }
        
	private:
		friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            std::size_t s = base_type::size();
            ar & boost::serialization::make_nvp("population_size", s);
            for(const_iterator i=base_type::begin(); i!=base_type::end(); ++i) {
                ar & boost::serialization::make_nvp("individual", **i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            std::size_t s;
            ar & boost::serialization::make_nvp("population_size", s);
            for(std::size_t i=0; i<s; ++i) {
                individual_type j;
                ar & boost::serialization::make_nvp("individual", j);
                individual_ptr_type p(new individual_type(j));
                base_type::push_back(p);
            }
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();		
	};

} // ea

#endif
