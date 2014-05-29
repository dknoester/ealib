/* digital_evolution/position.h
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

#ifndef _EA_DIGITAL_EVOLUTION_POSITION_H
#define _EA_DIGITAL_EVOLUTION_POSITION_H

#include <vector>

namespace ealib {
    
    //! Dimensions of a position vector.
    enum position_dimension { XPOS=0, YPOS=1, HEADING=2 };
    
    /*! Type that is contained (and owned) by organisms to uniquely identify
     their location in the environment.
     
     \warning: This type must be serializable.
     */
    typedef std::vector<int> position_type;

    //! Returns a position vector constructed from x, y, and h (heading).
    template <typename T>
    position_type make_position(T x=-1, T y=-1, T h=-1) {
        position_type pos(3,0);
        pos[XPOS] = x; pos[YPOS] = y; pos[HEADING]=h;
        return pos;
    }

} // ealib

#endif
