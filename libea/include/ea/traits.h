/* traits.h
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
#ifndef _EA_TRAITS_H_
#define _EA_TRAITS_H_

#include <boost/serialization/nvp.hpp>
#include <ea/fitness_function.h>

namespace ealib {
    
    /*! Fitness trait.
     
     This trait type adds a fitness field to an individual.  As ealib::individual
     already contains a fitness value, this is meant more for subpopulations, as
     in a metapopulation EA.
     */
    template <typename T>
    struct fitness_trait {
        typedef typename T::fitness_type fitness_type;
        
        //! Returns the current fitness value.
        fitness_type& fitness() { return _fitness; }

        //! Returns the current fitness value (const-qualified).
        const fitness_type& fitness() const { return _fitness; }

        //! Serialize this trait.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("fitness_trait", _fitness);
        }

        fitness_type _fitness; //!< Fitness for the individual that holds this trait.
    };

    //! Empty traits type.
    template <typename T>
    struct null_trait {
        //! Serialize this trait.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
        }
    };

} // ea

#endif
