/* isolated_subpopulations.
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
#ifndef _EA_GENERATIONAL_MODELS_ISOLATED_SUBPOPULATIONS_H_
#define _EA_GENERATIONAL_MODELS_ISOLATED_SUBPOPULATIONS_H_

#include <ea/generational_model.h>

namespace ealib {
	namespace generational_models {
        
        /*! Default generational model for a metapopulation EA, where all
         subpopulations are updated in lock-step, and do not themselves engage in
         a subpopulation-level evolutionary process.
         
         This generational model can be used to support an island model GA.
         */
        struct isolated_subpopulations : public generational_model {
            //! Apply this generational model to the meta_population EA.
            template <typename Population, typename EA>
            void operator()(Population& population, EA& ea) {
                BOOST_CONCEPT_ASSERT((PopulationConcept<Population>));
                BOOST_CONCEPT_ASSERT((EvolutionaryAlgorithmConcept<EA>));
                for(typename Population::iterator i=population.begin(); i!=population.end(); ++i) {
                    (*i)->repr().update();
                }
            }
        };
        
	} // generational_models
} // ea

#endif
