/* digital_evolution/utils/configurable_mutation.h
 *
 * This file is part of EALib.
 *
 * Copyright 2012 David B. Knoester, Heather J. Goldsby.
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


#ifndef _EA_DIGITAL_EVOLUTION_UTILS_CONFIGURABLE_MUTATION_H_
#define _EA_DIGITAL_EVOLUTION_UTILS_CONFIGURABLE_MUTATION_H_


#include <ea/digital_evolution.h>
#include <ea/digital_evolution/hardware.h>
#include <ea/digital_evolution/instruction_set.h>
#include <ea/digital_evolution/discrete_spatial_environment.h>
#include <ea/datafiles/reactions.h>
#include <ea/cmdline_interface.h>
#include <ea/metapopulation.h>
#include <ea/selection/random.h>
#include <ea/mutation.h>

using namespace ealib;


/*! Mutation - Per-site mutation at a configurable rate
 */
struct configurable_per_site {            
    typedef mutation::site::uniform_integer mutation_type;
    
    configurable_per_site(double prob) : _mp(prob) {
    }
    
    //! Iterate through all elements in the given representation, possibly mutating them.
    template <typename EA>
    void operator()(typename EA::individual_type& ind, EA& ea) {
        typename EA::representation_type& repr=ind.repr();
        for(typename EA::representation_type::iterator i=repr.begin(); i!=repr.end(); ++i){
            if(ea.rng().p(_mp)) {
                _mt(i, ea);
            }
        }
    }
    
    mutation_type _mt;
    double _mp; //! Mutation probability
};


#endif
