/* attributes.h
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
