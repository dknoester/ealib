/* digital_evolution/extra_instruction_set/matrix.h
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

#ifndef _EA_DIGITAL_EVOLUTION_EXTRA_INSTRUCTION_SETS_MATRIX_H_
#define _EA_DIGITAL_EVOLUTION_EXTRA_INSTRUCTION_SETS_MATRIX_H_

/* These instructions turn a torroid into a matrix. */


namespace ealib {
    
    namespace instructions {
        
        //! Detects matrix edges -- 1 if edge; 0 otherwise
        template<typename EA>
        bool matrix_edge(EA& ea,
                         typename EA::environment_type::location_type me,
                         typename EA::environment_type::location_type you) {
            
            int me_x = me.r[0];
            int me_y = me.r[1];
            
            int max_x = get<SPATIAL_X>(ea) - 1;
            int max_y = get<SPATIAL_Y>(ea) - 1;
            
            int you_x = you.r[0];
            int you_y = you.r[1];
            
            if (((me_x == 0 ) && (you_x == max_x)) ||
                ((me_x == max_x) && (you_x == 0)) ||
                ((me_y == 0) && (you_y == max_y)) ||
                ((me_y == max_y) && (you_y == 0))) {
                return true;
            } else {
                return false;
            }
            
        }
        
        //! Get whether a neighboring organism exists.
        DIGEVO_INSTRUCTION_DECL(is_neighbor_matrix) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p);
            
            // check to make sure we aren't peeking around an edge
            if (matrix_edge(ea, ea.env().location(p->position()), l)) {
                return;
            }
            
            if(l.occupied()) {
                hw.setRegValue(hw.modifyRegister(), 1);
            } else {
                hw.setRegValue(hw.modifyRegister(), 0);
            }
        }

        //! Send a message to the currently-faced neighbor.
        DIGEVO_INSTRUCTION_DECL(tx_msg_matrix) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p);
            // check to make sure we aren't peeking around an edge
            if (matrix_edge(ea, ea.env().location(p->position()), l)) {
                return;
            }
            
            if(l.occupied()) {
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
            }
        }
        
        //! Send a message to the currently-faced neighbor.
        DIGEVO_INSTRUCTION_DECL(tx_msg_check_task_matrix) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p);
            
            // check to make sure we aren't peeking around an edge
            if (matrix_edge(ea, ea.env().location(p->position()), l)) {
                return;
            }
            
            if(l.occupied()) {
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
            }
            p->outputs().push_front(hw.getRegValue(hw.modifyRegister()));
            p->outputs().resize(1);
            ea.tasklib().check_tasks(*p,ea);
        }

        //! Broadcast a message.
        DIGEVO_INSTRUCTION_DECL(bc_msg_matrix) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            
            typedef typename EA::environment_type::neighborhood_iterator neighborhood_iterator;
            std::pair<neighborhood_iterator,neighborhood_iterator> ni=ea.env().neighborhood(*p);
            for(; ni.first!=ni.second; ++ni.first) {
                typename EA::environment_type::location_type& l=*ni.first;
                // check to make sure we aren't peeking around an edge
                if (matrix_edge(ea, ea.env().location(p->position()), l)) {
                    continue;
                }
                
                if(l.occupied()) {
                    l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
                }
            }
        }

        
        //! Detect matrix edge: 1=edge; 0=otherwise
        DIGEVO_INSTRUCTION_DECL(on_edge_matrix) {
            int rbx = hw.modifyRegister();
            
            int my_x = (p->position()).r[0];
            int my_y = (p->position()).r[1];
            int edge = 0;
            
            if (my_x == 0 ||
                my_x == (get<SPATIAL_X>(ea) - 1) ||
                my_y == 0 ||
                my_y == (get<SPATIAL_Y>(ea) - 1) ) {
                edge = 1;
            }
            
            hw.setRegValue(rbx,edge);
        }
        

        
        
    } // instructions
} // ea

#endif
