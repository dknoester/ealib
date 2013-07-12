/* action.h
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
#ifndef _EA_LEARNING_CLASSIFIER_ACTION_H_
#define _EA_LEARNING_CLASSIFIER_ACTION_H_

#include <ea/meta_data.h>

namespace ealib {
    
    //! Maximum number of individuals that will act.
    LIBEA_MD_DECL(LCS_ACTION_N, "ea.lcs.action_n", std::size_t);
    
    //! Compare (pointers to) individuals based on the natural order of their fitnesses in ascending order.
    template <typename EA>
    struct bid_comparator {
        //! Returns true if fitness(x) < fitness(y), false otherwise.
        bool operator()(typename EA::individual_ptr_type x, typename EA::individual_ptr_type y) {
            return x->repr().bid < y->repr().bid;
        }
    };
    
    struct default_action {
        
        template <typename EA>
        void operator()(typename EA::population_type& matchset, typename EA::population_type& actionset, typename EA::message_board_type& mb, EA& ea) {
            // sort the matchset based on their bids:
            std::sort(matchset.begin(), matchset.end(), bid_comparator<EA>());
            
            // select the n highest bids for the actionset:
            std::size_t actors=get<LCS_ACTION_N>(ea,1);
            std::copy(matchset.rbegin(), matchset.rbegin()+actors, std::back_inserter(actionset));
            
            // post messages from each individual in the action set:
            for(typename EA::population_type::iterator i=actionset.begin(); i!=actionset.end(); ++i) {
                mb.insert(mb.end(), (*i)->repr().action_message);
            }
        }
    };

} // ea

#endif
