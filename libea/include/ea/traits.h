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
#ifndef _EA_TRAITS_H_
#define _EA_TRAITS_H_

#include <boost/serialization/nvp.hpp>
#include <ea/meta_data.h>
#include <ea/fitness_function.h>
#include <ea/phenotype.h>
#include <ea/line_of_descent.h>

namespace ealib {
    
    
    /*! Default traits for individuals in an evolutionary algorithm.
     
     The default traits include a bit more than what might be strictly
     necessary, just to limit the number of hoops that need to be jumped
     through when doing fairly common things.  I.e., they include fitness,
     phenotype, and line of descent attributes.  Of these, only fitness is
     currently serialized.  Note that this has the ramification that we do
     NOT save LoD information across checkpoints.
     */
    template <typename T>
    struct default_traits : traits::fitness_trait<T>, traits::phenotype_trait<T> {
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("fitness_trait", boost::serialization::base_object<traits::fitness_trait<T> >(*this));
        }
    };
    
    template <typename T>
    struct default_lod_traits : traits::fitness_trait<T>, traits::phenotype_trait<T>, traits::lod_traits<T> {
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("fitness_trait", boost::serialization::base_object<traits::fitness_trait<T> >(*this));
        }
    };
    
    
    template <typename T>
    struct null_traits {
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
        }
    };
    
    namespace access {
        
        //! Accessor for an individual's traits.
        struct traits {
            template <typename EA>
            typename EA::individual_type::traits_type operator()(typename EA::individual_type& ind, EA& ea) {
                return ind.traits();
            }
        };
        
        //! Fitness accessor functor.
        struct fitness {
            template <typename EA>
            typename EA::individual_type::fitness_type& operator()(typename EA::individual_type& ind, EA& ea) {
                return ealib::fitness(ind,ea);
            }
        };
        
        //! Accessor for a specific element of meta-data from an individual.
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
