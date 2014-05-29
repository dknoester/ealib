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
#ifndef _EA_POPULATION_STRUCTURE_H_
#define _EA_POPULATION_STRUCTURE_H_

//#include <boost/iterator/indirect_iterator.hpp>
//#include <boost/serialization/nvp.hpp>
//#include <boost/serialization/vector.hpp>
//#include <boost/serialization/split_member.hpp>
//#include <boost/shared_ptr.hpp>
//
//#include <ea/concepts.h>
//#include <ea/meta_data.h>


namespace ealib {
        
    //! Tag that indicates a single population is being used.
    struct singlePopulationS { };
    
    //! Tag that indicates multiple populations are being used.
    struct multiPopulationS { };

//    /*! Population; a container for individuals.
//	 */
//	template <typename T>
//	class ptr_vector : public std::vector<T> {
//	public:
//        typedef T individual_ptr_type;
//        typedef typename T::element_type individual_type;
//		
//		typedef std::vector<individual_ptr_type> parent;
//        typedef typename parent::value_type value_type;
//		typedef typename parent::iterator iterator;
//		typedef typename parent::const_iterator const_iterator;
//		typedef typename parent::reverse_iterator reverse_iterator;
//		
//		//! Constructor.
//		ptr_population() {
//		}
//        
//        //! Initializing constructor.
//        ptr_population(std::size_t n) : parent(n) {
//        }
//
//        //! Initializing constructor.
//        ptr_population(std::size_t n, const value_type& t) : parent(n,t) {
//        }
//
//        //! Initializing constructor.
//        template <typename ForwardIterator>
//        ptr_population(ForwardIterator f, ForwardIterator l) : parent(f,l) {
//        }
//        
//        //! Operator ==
//        bool operator==(const ptr_population& that) {
//            typedef boost::indirect_iterator<const_iterator> ici;
//            return (parent::size() == that.size())
//            && std::equal(ici(parent::begin()), ici(parent::end()), ici(that.begin()));
//        }
//        
//        //! Destructor.
//        virtual ~ptr_population() {
//        }
//        
//	private:
//		friend class boost::serialization::access;
//        
//		template<class Archive>
//		void save(Archive & ar, const unsigned int version) const {
//            std::size_t s = parent::size();
//            ar & boost::serialization::make_nvp("population_size", s);
//            for(const_iterator i=parent::begin(); i!=parent::end(); ++i) {
//                ar & boost::serialization::make_nvp("individual", **i);
//            }
//		}
//		
//		template<class Archive>
//		void load(Archive & ar, const unsigned int version) {
//            std::size_t s;
//            ar & boost::serialization::make_nvp("population_size", s);
//            for(std::size_t i=0; i<s; ++i) {
//                individual_ptr_type p(new individual_type());
//                ar & boost::serialization::make_nvp("individual", *p);
//                parent::push_back(p);
//            }
//		}
//		BOOST_SERIALIZATION_SPLIT_MEMBER();		
//	};

} // ea

#endif
