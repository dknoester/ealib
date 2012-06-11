/* artificial_life/artificial_life.h 
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

#ifndef _EA_HARDWARE_ISA_H_
#define _EA_HARDWARE_ISA_H_

#include <boost/shared_ptr.hpp>
#include <vector>

#include <ea/artificial_life/instructions.h>

namespace ea {
        
        template <typename Hardware, typename Organism, typename AL>
        class isa {
        public: 
            typedef Hardware hardware_type;
            typedef AL al_type;
            typedef typename al_type::individual_ptr_type individual_ptr_type;
            
            typedef abstract_instruction<hardware_type,al_type> inst_type;
            typedef std::vector<boost::shared_ptr<inst_type> > isa_type;
            
            isa() {
                append<inst_nop_a>();
                append<inst_nop_b>();
                append<inst_nop_c>();
                append<inst_nop_x>();
                append<inst_mov_head>();
                append<inst_if_label>();
                append<inst_h_search>();
                append<inst_nand>();
                append<inst_input>();
                append<inst_output>();
                append<inst_repro>();
            }
            
            template <template <typename,typename> class Instruction>
            void append() {
                boost::shared_ptr<inst_type> p(new Instruction<hardware_type,al_type>());
                _isa.push_back(p);
            }            
            
            bool operator()(std::size_t inst, hardware_type& hw, individual_ptr_type p, al_type& al) {
                return (*_isa[inst])(hw,p,al);
            }
            
            bool is_nop(std::size_t inst) {
                return _isa[inst]->is_nop();
            }
            
        protected:
            isa_type _isa; //!< ...
        };        

} // ea

#endif
