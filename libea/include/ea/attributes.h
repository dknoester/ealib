#ifndef _EA_ATTRIBUTES_H_
#define _EA_ATTRIBUTES_H_

#include <boost/serialization/nvp.hpp>


namespace ea {

    //! Base class for individual attributes.
    template <typename EA>
    struct individual_attributes {
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
        }
    };
    
    namespace attributes {
        
        //! Accessor for individual fitness.
        struct fitness {
            template <typename Individual>
            typename Individual::fitness_type& operator()(Individual& x) {
                return x.fitness();
            }
        };
        
    } // comparators
} // ea

#endif
