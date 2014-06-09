/* periodic_competition.h
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
#ifndef _EA_GENERATIONAL_MODELS_PERIODIC_COMPETITION_H_
#define _EA_GENERATIONAL_MODELS_PERIODIC_COMPETITION_H_

#include <ea/algorithm.h>
#include <ea/metadata.h>
#include <ea/generational_models/isolated_subpopulations.h>
#include <ea/generational_models/moran_process.h>

namespace ealib {
    LIBEA_MD_DECL(METAPOP_COMPETITION_PERIOD, "ea.metapopulation.competition_period", unsigned int);

	namespace generational_models {
        
        /*! This is a metapopulation-specific generational model that provides 
         periodic competition among subpopulations.
         
         It takes as template parameters an update model and a competition model.
         The update model is used to update each subpopulation, while the
         competition model, predictably, competes these subpopulations with each
         other.
         
         \warning Fitness is only calculated by the competition model; if fitness
         of a subpopulation can change over time, then it must be nonstationary.
         */
        template
        < typename UpdateModel=isolated_subpopulations
        , typename CompetitionModel=moran_process< >
        > struct periodic_competition {
            typedef UpdateModel update_model_type;
            typedef CompetitionModel competition_model_type;
            
            //! Apply this generational model to the metapopulation.
            template <typename Population, typename MEA>
            void operator()(Population& population, MEA& mea) {
                _um(population, mea);

                // periodic competition:
                if(periodic<METAPOP_COMPETITION_PERIOD>(mea.current_update(), mea)) {
                    _cm(population, mea);
                }
            }
            
            update_model_type _um;
            competition_model_type _cm;
        };
        
	} // generational_models
} // ea

#endif
