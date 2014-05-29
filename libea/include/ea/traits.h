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
#include <ea/line_of_descent.h>

namespace ealib {
    
    /*! Default traits for individuals in an evolutionary algorithm.
     
     Traits are defined as runtime information that is attached to individuals
     in an EA.  For example, pointers to a phenotype or a line of descent.  Traits
     may be serializable.
     
     The default traits type simply provides a field for fitness.
     */
    template <typename T>
    struct default_ea_traits {
        typedef typename T::fitness_type fitness_type;
        
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("fitness", _fitness);
        }
        
        //! Returns the current fitness value.
        fitness_type& fitness() { return _fitness; }

        //! Returns the current fitness value (const-qualified).
        const fitness_type& fitness() const { return _fitness; }
        
        fitness_type _fitness;
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
            typename EA::fitness_type& operator()(typename EA::individual_type& ind, EA& ea) {
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
