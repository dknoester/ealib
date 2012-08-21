/* digital_evolution/ancestors.h 
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

#ifndef _EA_DIGITAL_EVOLUTION_ANCESTORS_H_
#define _EA_DIGITAL_EVOLUTION_ANCESTORS_H_

#include <algorithm>
#include <ea/meta_data.h>


namespace ea {
    
    //! Generates a representation for a repro ancestor.
    struct repro_ancestor {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            typename EA::representation_type repr;
            repr.resize(get<REPRESENTATION_SIZE>(ea));
            std::fill(repr.begin(), repr.end(), ea.isa()["nop_x"]);
            *repr.rbegin() = ea.isa()["repro"];
            return repr;
        }
    };
    
    //! Generates a representation for a nop-x ancestor.
    struct nopx_ancestor {
        template <typename EA>
        typename EA::representation_type operator()(EA& ea) {
            typename EA::representation_type repr;
            repr.resize(get<REPRESENTATION_SIZE>(ea));
            std::fill(repr.begin(), repr.end(), ea.isa()["nop_x"]);
            return repr;
        }
    };
    
} // ea

#endif
