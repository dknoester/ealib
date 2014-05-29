/* population_structure.h
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
#ifndef _EA_POPULATION_STRUCTURE_H_
#define _EA_POPULATION_STRUCTURE_H_

namespace ealib {
    
    /* There are a variety of different parts of an EA that depend upon the
     underlying population structure.  These tag structs are to be attached
     to EAs to indicate whether an EA is a single population or a 
     multipopulation EA.
     */
    
    //! Tag that indicates a single population is being used.
    struct singlePopulationS { };
    
    //! Tag that indicates multiple populations are being used.
    struct multiPopulationS { };

} // ea

#endif
