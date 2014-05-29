/* shared_ptr_vector.h
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
#ifndef _EA_DATA_STRUCTURES_SHARED_PTR_VECTOR_H_
#define _EA_DATA_STRUCTURES_SHARED_PTR_VECTOR_H_

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/serialization/nvp.hpp>
#include <vector>

namespace ealib {

    /*! This class is a specialization of std::vector that assumes its members
     are boost::shared_ptr's.  It is mostly a convenience class for supporting
     serialization.
	 */
	template <typename T>
	class shared_ptr_vector : public std::vector<T> {
	public:
        typedef typename T::element_type element_type;
		typedef std::vector<T> parent;
        typedef typename parent::value_type value_type;
		typedef typename parent::iterator iterator;
		typedef typename parent::const_iterator const_iterator;
		typedef typename parent::reverse_iterator reverse_iterator;
		
		//! Constructor.
		shared_ptr_vector() {
		}
        
        //! Initializing constructor.
        shared_ptr_vector(std::size_t n) : parent(n) {
        }

        //! Initializing constructor.
        shared_ptr_vector(std::size_t n, const value_type& t) : parent(n,t) {
        }

        //! Initializing constructor.
        template <typename ForwardIterator>
        shared_ptr_vector(ForwardIterator f, ForwardIterator l) : parent(f,l) {
        }
        
        //! Operator ==
        bool operator==(const shared_ptr_vector& that) {
            typedef boost::indirect_iterator<const_iterator> ici;
            return (parent::size() == that.size())
            && std::equal(ici(parent::begin()), ici(parent::end()), ici(that.begin()));
        }
        
        //! Destructor.
        virtual ~shared_ptr_vector() {
        }
        
	private:
		friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
            std::size_t s = parent::size();
            ar & boost::serialization::make_nvp("size", s);
            for(const_iterator i=parent::begin(); i!=parent::end(); ++i) {
                ar & boost::serialization::make_nvp("element", **i);
            }
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
            std::size_t s;
            ar & boost::serialization::make_nvp("size", s);
            for(std::size_t i=0; i<s; ++i) {
                value_type p(new element_type());
                ar & boost::serialization::make_nvp("element", *p);
                parent::push_back(p);
            }
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();		
	};

} // ea

#endif
