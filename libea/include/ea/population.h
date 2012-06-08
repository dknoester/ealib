#ifndef _EA_POPULATION_H_
#define _EA_POPULATION_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <ea/interface.h>
#include <ea/concepts.h>
#include <ea/meta_data.h>


namespace ea {
    
    /*! Random access container of individuals.
	 */
	template <typename Individual, typename IndividualPtr>
	class population : public std::vector<IndividualPtr> {
	public:
        typedef Individual individual_type;
        typedef IndividualPtr individual_ptr_type;
		typedef population<individual_type, individual_ptr_type> population_type;
		
		typedef std::vector<individual_ptr_type> base_type;
        typedef typename base_type::value_type value_type;
		typedef typename base_type::iterator iterator;
		typedef typename base_type::const_iterator const_iterator;
		typedef typename base_type::reverse_iterator reverse_iterator;
		
		//! Constructor.
		population() {
			BOOST_CONCEPT_ASSERT((PopulationConcept<population_type>));
		}
        
        //! Destructor.
        virtual ~population() {
        }
        
        void append(value_type i) {
            base_type::push_back(i);
        }

        void append(iterator i) {
            base_type::push_back(*i);
        }

        template <typename ForwardIterator>
        void append(ForwardIterator f, ForwardIterator l) {
            base_type::insert(base_type::end(), f, l);
        }

        individual_type& ind(iterator i) {
            return **i;
        }
        
        const individual_type& ind(const_iterator i) const {
            return **i;
        }
        
        individual_ptr_type ptr(iterator i) {
            return *i;
        }

        const individual_ptr_type ptr(const_iterator i) {
            return *i;
        }

        individual_ptr_type ptr(value_type i) {
            return i;
        }

        template <typename EA>
        value_type make_population_entry(individual_ptr_type p, EA& ea) {
            return p;
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
