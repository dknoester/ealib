/* generational_model.h
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
#ifndef _EA_GENERATIONAL_MODEL_H_
#define _EA_GENERATIONAL_MODEL_H_

#include <limits>
#include <cmath>
#include <boost/serialization/nvp.hpp>


namespace ealib {
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
