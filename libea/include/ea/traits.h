/* traits.h
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
#ifndef _EA_TRAITS_H_
#define _EA_TRAITS_H_

#include <boost/serialization/nvp.hpp>
#include <ea/meta_data.h>
#include <ea/fitness_function.h>
#include <ea/phenotype.h>
#include <ea/line_of_descent.h>

namespace ealib {
    
    /* Traits for individuals in an evolutionary algorithm.
     
     Traits are defined as non-serializable runtime-only information that is
     attached to individuals in an EA.  For example, pointers to a phenotype
     or line of descent.
     */
    template <typename T>
    struct default_traits : traits::phenotype_trait<T> {
    };
    
    template <typename T>
    struct default_lod_traits : traits::phenotype_trait<T>, traits::lod_trait<T> {
    };
    
    template <typename T>
    struct null_traits {
    };
    
    namespace access {
        
        //! Functor that returns an individual's traits.
        struct traits {
            template <typename EA>
            typename EA::individual_type::traits_type operator()(typename EA::individual_type& ind, EA& ea) {
                return ind.traits();
            }
        };
        
        //! Functor that returns an individual's fitness.
        struct fitness {
            template <typename EA>
            typename EA::individual_type::fitness_type& operator()(typename EA::individual_type& ind, EA& ea) {
                return ealib::fitness(ind,ea);
            }
        };
        
        //! Functor that returns an element of meta-data from an individual.
        template <typename MDType>
        struct meta_data {
            template <typename EA>
            typename MDType::value_type operator()(typename EA::individual_type& ind, EA& ea) {
                return ealib::get<MDType>(ind);
            }
        };
        
    } // access
} // ea

#endif
