#ifndef _EA_GENERATIONAL_MODEL_H_
#define _EA_GENERATIONAL_MODEL_H_

#include <limits>
#include <cmath>
#include <boost/serialization/nvp.hpp>


namespace ea {
    namespace generational_models {
        
        /*! Abstract generational model.
         */
        struct generational_model {
            unsigned long update;
            
            generational_model() : update(0) {
            }
            
            unsigned long current_update() const {
                return update;
            }
            
            void next_update() {
                ++update;
            }
            
            template <class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                ar & BOOST_SERIALIZATION_NVP(update);
            }
        };
        
    } // generational_models
} // ea

#endif
