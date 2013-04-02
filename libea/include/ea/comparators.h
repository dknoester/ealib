/* comparators.h
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
#ifndef _EA_COMPARATORS_H_
#define _EA_COMPARATORS_H_

#include <ea/attributes.h>

namespace ealib {
    namespace comparators {

        //! Compare individual pointers based on the natural order of their fitnesses in ascending order.
        struct fitness {
            template <typename IndividualPtr>
            bool operator()(IndividualPtr x, IndividualPtr y) {
                return ealib::fitness(*x) < ealib::fitness(*y);
            }
        };
        
        //! Compare individual pointers based on the natural order of their fitnesses in descending order.
        struct fitness_desc {
            template <typename IndividualPtr>
            bool operator()(IndividualPtr x, IndividualPtr y) {
                return ealib::fitness(*x) > ealib::fitness(*y);
            }
        };

      template <typename MDType>
      struct meta_data {
        template <typename IndividualPtr>
        bool operator()(IndividualPtr x, IndividualPtr y) {
          return ealib::get<MDType>(*x) < ealib::get<MDType>(*y);
        }
      };
      
        /*! Compare individuals based on the natural order of their m'th objective,
         which is useful with multivalued fitnesses.
         */
        struct objective {
            objective(std::size_t m) : _m(m) {
            }
            
            template <typename IndividualPtr>
            bool operator()(IndividualPtr& a, IndividualPtr& b) {
                return a->fitness()[_m] < b->fitness()[_m];
            }
            
            std::size_t _m;
        };
        
    } // comparators
} // ea

#endif
