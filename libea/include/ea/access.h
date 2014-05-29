/* access.h
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
#ifndef _EA_ACCESS_H_
#define _EA_ACCESS_H_

#include <ea/meta_data.h>
#include <ea/fitness_function.h>
#include <ea/traits.h>


namespace ealib {
    namespace access {

        /* This namespace contains functors that can be used to access different
         values within an individual or EA.  These are most useful as template
         paramenters to selection strategies.
         */
        
        //! Functor that returns an individual's traits object.
        struct traits {
            template <typename EA>
            typename EA::traits_type operator()(typename EA::individual_type& ind, EA& ea) {
                return ind.traits();
            }
        };
        
        //! Functor that returns an individual's fitness object.
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
