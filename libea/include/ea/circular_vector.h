#ifndef _EA_CIRCULAR_VECTOR_H_
#define _EA_CIRCULAR_VECTOR_H_

#include <vector>

namespace ea {

	template <typename ForwardIterator>
    class circular_iterator {
    
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
