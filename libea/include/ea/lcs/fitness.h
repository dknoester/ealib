/* fitness.h
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
#ifndef _EA_LEARNING_CLASSIFIER_FITNESS_H_
#define _EA_LEARNING_CLASSIFIER_FITNESS_H_

#include <ea/fitness_function.h>
#include <ea/meta_data.h>

namespace ealib {

    LIBEA_MD_DECL(LCS_REWARD, "individual.lcs.reward", double);
    
    /*! Placeholder for an LCS fitness function that rewards individual classifiers
     for having high accuracy, a la XCS.
     */
    struct accuracy_fitness : public fitness_function<unary_fitness<double,minimizeS>, nonstationaryS, deterministicS> {
        template <typename Individual, typename EA>
        double operator()(Individual& ind, EA& ea) {
            return get<LCS_REWARD>(ind,0.0);
        }
    };
    
} // ea

#endif

