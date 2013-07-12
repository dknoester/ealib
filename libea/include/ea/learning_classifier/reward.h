/* reward.h
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
#ifndef _EA_LEARNING_CLASSIFIER_REWARD_H_
#define _EA_LEARNING_CLASSIFIER_REWARD_H_

namespace ealib {
    
    /*! The reward function monitors the state of the environment and adjusts the "strength"
     of classifiers in the action set (and potentially past action sets).  The reward
     function also implements learning, via bucket brigade or Q-learning among chains
     of classifiers.
     */
    struct abstract_reward {
        template <typename EA>
        void operator()(typename EA::environment_type& env, typename EA::population_type& actionset, EA& ea) {
        }
    };
    
} // ea

#endif
