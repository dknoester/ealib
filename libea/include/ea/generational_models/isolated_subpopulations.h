/* isolated_subpopulations.
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
#ifndef _EA_GENERATIONAL_MODELS_ISOLATED_SUBPOPULATIONS_H_
#define _EA_GENERATIONAL_MODELS_ISOLATED_SUBPOPULATIONS_H_

namespace ealib {
	namespace generational_models {
        
        /*! Default generational model for a metapopulation EA, where all
         EAs are updated in lock-step, and do not themselves engage in
         a subpopulation-level evolutionary process.
         
         Coupled with a migration event, this generational model provides an
         island model.
         */
        struct isolated_subpopulations {
            //! Apply this generational model to the metapopulation.
            template <typename Population, typename EA>
            void operator()(Population& population, EA& ea) {
                for(typename Population::iterator i=population.begin(); i!=population.end(); ++i) {
                    (*i)->update();
                }
            }
        };
        
	} // generational_models
} // ea

#endif
