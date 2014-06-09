/* island_model.h
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
#ifndef _EA_ISLAND_MODEL_H_
#define _EA_ISLAND_MODEL_H_

#include <boost/tuple/tuple.hpp>
#include <ea/events.h>
#include <ea/metadata.h>

namespace ealib {
    LIBEA_MD_DECL(ISLAND_MIGRATION_PERIOD, "ea.island_model.migration_period", int);
    LIBEA_MD_DECL(ISLAND_MIGRATION_RATE, "ea.island_model.migration_rate", double);
    
    /*! Island models provide for migration among different populations in a
     meta-population EA.
     */
    template <typename MEA>
    struct island_model : periodic_event<ISLAND_MIGRATION_PERIOD,MEA> {
        //! Constructor.
        island_model(MEA& ea) : periodic_event<ISLAND_MIGRATION_PERIOD,MEA>(ea) {
        }
        
        //! Destructor.
        virtual ~island_model() {
        }
        
        //! Perform migration of individuals among populations.
        virtual void operator()(MEA& ea) {
            int migrations = static_cast<int>(get<ISLAND_MIGRATION_RATE>(ea)*get<POPULATION_SIZE>(ea)*get<METAPOPULATION_SIZE>(ea));
            
            // technically, this should probably issue a warning?
            if((migrations == 0) || (get<METAPOPULATION_SIZE>(ea) == 1)) {
                return;
            }
            
            for( ; migrations>0; --migrations) {
                typename MEA::iterator s,t; // source and target populations
                boost::tie(s,t) = ea.rng().choose_two_range(ea.begin(), ea.end());
                typename MEA::individual_type::iterator migrant=ea.rng().choice(s->begin(), s->end()); // migrating individual
                t->insert(t->end(),t->copy_individual(*migrant)); // copy the migrant to the target population
                s->erase(migrant); // remove the migrant from the source population
            }
        }
    };
    
} // ea

#endif
