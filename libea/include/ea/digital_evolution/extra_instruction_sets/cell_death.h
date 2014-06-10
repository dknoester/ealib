/* digital_evolution/extra_instruction_set/cell_death.h
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

#ifndef _EA_DIGITAL_EVOLUTION_EXTRA_INSTRUCTION_SETS_CELL_DEATH_H_
#define _EA_DIGITAL_EVOLUTION_EXTRA_INSTRUCTION_SETS_CELL_DEATH_H_

namespace ealib {
    
    LIBEA_MD_DECL(APOPTOSIS_STATUS, "ea.digevo.apoptosis_status", int);
    
    namespace instructions {
        
        //! Apaptosis (triggers death) instruction.
        DIGEVO_INSTRUCTION_DECL(apoptosis) {
            p->alive() = false;
            ea.events().death(*p,ea);
            put<APOPTOSIS_STATUS>(1, *p);
        }
        
    } // instructions
} // ea

#endif
