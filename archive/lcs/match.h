/* match.h
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
#ifndef _EA_LEARNING_CLASSIFIER_MATCH_H_
#define _EA_LEARNING_CLASSIFIER_MATCH_H_

#include <algorithm>
#include <ea/algorithm.h>
#include <ea/metadata.h>

namespace ealib {

    /*! Default message matching operator.
     
     Assumes that the first message-sized bits in the representation are the match
     string.
     */
    struct default_match {

        template <typename EA>
        bool matches(typename EA::message_type& msg, typename EA::individual_type& ind, EA& ea) {
            assert(msg.size() == ind.repr().match_string.size());
            return std::equal(msg.begin(), msg.end(), ind.repr().match_string.begin(), algorithm::tdc_binary_predicate());
        }
        
        template <typename EA>
        void operator()(typename EA::message_board_type& mb, typename EA::population_type& population, typename EA::population_type& matchset, EA& ea) {
            for(typename EA::message_board_type::iterator i=mb.begin(); i!=mb.end(); ++i) {
                for(typename EA::population_type::iterator j=population.begin(); j!=population.end(); ++j) {
                    if(matches(*i,**j,ea)) {
                        i->consumed_by(*j);
                        matchset.insert(matchset.end(), *j);
                    }
                }
            }
        }
    };

} // ea

#endif
