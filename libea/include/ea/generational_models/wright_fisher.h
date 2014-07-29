/* wright_fisher.h
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
#ifndef _EA_GENERATIONAL_MODELS_WRIGHT_FISHER_H_
#define _EA_GENERATIONAL_MODELS_WRIGHT_FISHER_H_

#include <ea/access.h>
#include <ea/generational_models/generational.h>
#include <ea/selection/proportionate.h>

namespace ealib {
	namespace generational_models {
		
        /*! Wright-Fisher generational model.

         This generational model selects parents from the existing population
         proportionally to their fitness, recombines them to produce offspring, and 
         then the offspring are mutated and replace the parents.
         */
        template <typename AttributeAccessor=access::fitness>
        struct wright_fisher : generational<selection::proportionate<AttributeAccessor> > {
        };
		
	} // generational_models
} // ea

#endif
