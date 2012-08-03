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

#ifndef _EA_HARDWARE_AVIDA_INSTRUCTIONS_H_
#define _EA_HARDWARE_AVIDA_INSTRUCTIONS_H_

#include <iostream>
#include <functional>
#include <vector>
#include <utility>

namespace ea {
    
    template <typename Hardware, typename AL>
    struct abstract_instruction {
        //! The return value specifies the number of cycles executing this instruction costs.
        virtual int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) = 0;
    };
    
    //! Push nop-a onto the label stack        
    template <typename Hardware, typename AL>
    struct inst_nop_a : abstract_instruction<Hardware,AL> {
        virtual int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            hw.pushLabelStack(Hardware::NOP_A);
            return 0;
        }
    };
    
    //! Push nop-b onto the label stack
    template <typename Hardware, typename AL>
    struct inst_nop_b : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            hw.pushLabelStack(Hardware::NOP_B);
            return 0;
        }
    };
    
    //! Push nop-c onto the label stack
    template <typename Hardware, typename AL>
    struct inst_nop_c : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            hw.pushLabelStack(Hardware::NOP_C);
            return 0;
        }
    };
    
    //! Do nothing
    template <typename Hardware, typename AL>
    struct inst_nop_x : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            hw.clearLabelStack();
            return 1;
        }
    };
    
    /*! Allocate memory for this organism's offspring.
     
     Extend the organism's memory by 150%, set the read head to the beginning
     of its memory, and set the write head to the beginning of the newly-
     allocated space.  This instruction only has effect once per lifetime.
     */
    template <typename Hardware, typename AL>
    struct inst_h_alloc : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            hw.extendMemory();
            return 1;
        }
    };
    
    /*! Copy an instruction from the read head to the write head.
     
     The instruction currently pointed to by the read head is copied to the
     current position of the write head.  The write and read heads are then
     each advanced one instruction.
     */
    template <typename Hardware, typename AL>
    struct inst_h_copy : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            typename Hardware::representation_type& r = hw.repr();
            r[Hardware::WH] = r[Hardware::RH];
            hw.advanceHead(Hardware::WH);
            hw.advanceHead(Hardware::RH);            
            return 1;
        }
    };
    
    
    //! Move the ?IP? head to the same position as the flow control head
    template <typename Hardware, typename AL>
    struct inst_mov_head : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int h = hw.modifyHead();
            hw.setHeadLocation(h, hw.getHeadLocation(Hardware::FH));
            
            // If we moved the IP, we have to back up one instruction  
            // because of the auto-increment.
            if (h == Hardware::IP) {
                hw.advanceHead(h, -1);
            }
            hw.clearLabelStack();
            return 1;
        }
    };
    
    
    /*! Execute the next instruction if the complement was just copied.
     If there is no preceding label, or the complementary label was not
     just copied, skip the next instruction.
     */
    template <typename Hardware, typename AL>
    struct inst_if_label : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            if (!hw.isLabelStackEmpty()) {
                // what immediately preceeds the write head...
                int wh = hw.advance(Hardware::WH, -1);
                std::deque<int> label_comp = hw.getLabelComplement();
                // check through label in reverse order...
                // most recent label is on the back...
                for(int i=(label_comp.size() - 1);  i>=0; --i) { 
                    if(label_comp[i] != static_cast<int>(hw.repr()[wh])) {
                        hw.advanceHead(Hardware::IP);
                        hw.clearLabelStack();
                        return 1;
                    }
                    int wh = hw.advance(wh, -1);
                }
            } else {
                hw.advanceHead(Hardware::IP);
            }
            hw.clearLabelStack();
            return 1;
        }
    };
    
    /*! Search forward until the first complement to the label is found.
     
     From the current IP, scan forward in the memory (wrapping around as
     needed) looking for the first complement to the label.  If a complement
     is found, BX is set to the distance to the complement, CX is set to
     its size, and the flow head is set to the instruction immediately 
     following the complement.  If no label is found, BX and CX are set to
     zero, and the flow head is moved to the instruction following h_search.
     */
    template <typename Hardware, typename AL>
    struct inst_h_search : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            std::pair<int, int> comp_label = hw.findComplementLabel();
            hw.setHeadLocation(Hardware::FH, hw.getHeadLocation(Hardware::IP)); 
            int dist = comp_label.first; 
            // if the complement of the label exists...
            if (comp_label.first != -1) {
                hw.setRegValue(Hardware::BX, comp_label.first); 
                hw.setRegValue(Hardware::CX, comp_label.second);                
            } else { 
                hw.setRegValue(Hardware::BX, 0); 
                hw.setRegValue(Hardware::CX, 0);
                dist = 1;
            }
            hw.advanceHead(Hardware::FH, dist + comp_label.second);
            hw.clearLabelStack();
            return 1;
        }
    };
    

    /*! Divide this organism's memory between parent and offspring.
     
     Instructions from the beginning of the organism's memory to the current
     position of the read head are preserved for the parent, while instructions
     between the read head and the write head are split off to form the
     offspring's genome; the offspring is then ``born.''
     */
    template <typename Hardware, typename AL>
    struct h_divide : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            /* 
             sequence = org.memory[org.heads[org.RH]:org.heads[org.WH]]
             if org.check_divide(sequence):
             sequence = org.eve.variation.indel_mutation(sequence)
             org.genome.make_organism(sequence, org)
             # Now do the parent reset.
             # In an interesting twist, we need to back up the IP by one instruction.
             org.__init__(org.eve, org.genome)
             org.advance_head(org.IP, -1)
             */
            if(hw.age() >= (0.8 * hw.repr().size())) {            
                typename Hardware::representation_type& r=hw.repr();
                typename Hardware::representation_type offr(&r[hw.getHeadLocation(Hardware::RH)],
                                                            &r[hw.getHeadLocation(Hardware::WH)+1]);
                replicate(p, offr, al);
                hw.initialize();
            }
            return 1;
        }
    };
    
    /*! Read a new input into ?BX?.
     */
    template <typename Hardware, typename AL>
    struct inst_input : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int reg=hw.modifyRegister();
            
            if(p->inputs().size() == 2) {
                hw.setRegValue(reg, p->inputs().front());
                p->inputs().push_back(p->inputs().front());
                p->inputs().pop_front();
            } else {
                hw.setRegValue(reg, al.env().read(*p, al));
                p->inputs().push_front(hw.getRegValue(reg));
            }
            hw.clearLabelStack();
            return 1;
        }
    };

    
    /*! Output ?BX?.
     
     Executing this instruction triggers task evaluation on this output value
     and the last two input values.  Regardless of the specific cataylst type,
     fitness changes from multiple tasks are multiplied together.
     */
    template <typename Hardware, typename AL>
    struct inst_output : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            p->outputs().push_front(hw.getRegValue(hw.modifyRegister()));
            p->outputs().resize(1);
            al.tasklib().check_tasks(*p,al);
            hw.clearLabelStack();
            return 1;
        }
    };

    
    /*! Reproduce this organism.
     */
    template <typename Hardware, typename AL>
    struct inst_repro : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            if(hw.age() >= (0.8 * hw.repr().size())) {            
                replicate(p, hw.repr(), al);
                hw.initialize();
            }
            return 1;
        }
    };

    
    /*! Store BX nand CX into the ?BX? register.
     For this operation, BX and CX are treated as integers.
     */
    template <typename Hardware, typename AL>
    struct inst_nand : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int bxVal = hw.getRegValue(Hardware::BX);
            int cxVal = hw.getRegValue(Hardware::CX);
            int nandVal = ~(bxVal & cxVal);
            hw.setRegValue(hw.modifyRegister(), nandVal);
            hw.clearLabelStack();
            return 1;
        }
    };
    
    template <typename Hardware, typename AL>
    struct inst_push : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int bxVal = hw.getRegValue(hw.modifyRegister());
            hw.push_stack(bxVal);
            hw.clearLabelStack();
            return 1;
        }
    };
    
    template <typename Hardware, typename AL>
    struct inst_pop : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            if(!hw.empty_stack()) {
                hw.setRegValue(hw.modifyRegister(), hw.pop_stack());
                hw.clearLabelStack();
            }
            return 1;
        }
    };
    
    template <typename Hardware, typename AL>
    struct inst_swap : abstract_instruction<Hardware,AL> {
        int operator() (Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            
            int bxVal = hw.getRegValue(rbx);
            int cxVal = hw.getRegValue(rcx);

            hw.setRegValue(rbx, cxVal);
            hw.setRegValue(rcx, bxVal);
            hw.clearLabelStack();
            return 1;
        }
    };

    /*! Set the color of the organism's location.
     */
    template <typename Hardware, typename AL>
    struct inst_location_color : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int bxVal = hw.getRegValue(hw.modifyRegister());
            put<LOCATION_COLOR>(bxVal,*p->location());
            hw.clearLabelStack();
            return 1;
        }
    };
    
    /*!
     */
    template <typename Hardware, typename AL>
    struct inst_inc : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int rbx = hw.modifyRegister();
            hw.setRegValue(rbx, hw.getRegValue(rbx)+1);
            hw.clearLabelStack();
            return 1;
        }
    };

    /*!
     */
    template <typename Hardware, typename AL>
    struct inst_dec : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int rbx = hw.modifyRegister();
            hw.setRegValue(rbx, hw.getRegValue(rbx)-1);
            hw.clearLabelStack();
            return 1;
        }
    };

    /*!
     */
    template <typename Hardware, typename AL>
    struct inst_beacon : abstract_instruction<Hardware,AL> {
        int operator()(Hardware& hw, typename AL::individual_ptr_type p, AL& al) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);

            if((p->location()->x==0) && (p->location()->y==0)) {
                hw.setRegValue(rbx, 0);
                hw.setRegValue(rcx, 0);
            }
            
            hw.clearLabelStack();
            return 1;
        }
    };

    /*! Send a message to the currently-faced neighbor.
     */
    template <typename Hardware, typename EA>
    struct inst_tx_msg : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p,ea);
            
            if(l.occupied()) {
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
            }
            hw.clearLabelStack();
            return 1;
        }
    };
    
    /*! Send a message to the currently-faced neighbor.
     */
    template <typename Hardware, typename EA>
    struct inst_rx_msg : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            if(hw.msgs_queued()) {
                std::pair<int,int> msg = hw.pop_msg();
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                hw.setRegValue(rbx,msg.first);
                hw.setRegValue(rcx,msg.second);
            }
            hw.clearLabelStack();
            return 1;
        }
    };

    /*! Broadcast a message.
     */
    template <typename Hardware, typename EA>
    struct inst_bc_msg : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            
            typedef typename EA::environment_type::iterator iterator;
            std::pair<iterator,iterator> ni=ea.env().neighborhood(p,ea);
            for(; ni.first!=ni.second; ++ni.first) {
                typename EA::environment_type::location_type& l=*ni.first;
                if(l.occupied()) {
                    l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
                }
            }
            hw.clearLabelStack();
            return 1;
        }
    };

    //! Rotate the organism to the heading in ?bx?.
    template <typename Hardware, typename EA>
    struct inst_rotate : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            p->location()->set_heading(hw.getRegValue(hw.modifyRegister()));
            hw.clearLabelStack();
            return 1;
        }
    };
    
    //! Rotate the organism clockwise once.
    template <typename Hardware, typename EA>
    struct inst_rotate_cw : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            p->location()->alter_heading(-1);
            hw.clearLabelStack();
            return 1;
        }
    };

    /*! Rotate the organism counter-clockwise once.
     */
    template <typename Hardware, typename EA>
    struct inst_rotate_ccw : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            p->location()->alter_heading(1);
            hw.clearLabelStack();
            return 1;
        }
    };

    /*! Execute the next instruction if ?bx? < ?cx?.
     */
    template <typename Hardware, typename EA>
    struct inst_if_less : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            if(hw.getRegValue(rbx) >= hw.getRegValue(rcx)) {
                hw.advanceHead(Hardware::IP);
            }
            hw.clearLabelStack();
            return 1;
        }
    };

    /*! Donate any accumulated resource to this organism's group.
     */
    template <typename Hardware, typename EA>
    struct inst_donate_group : abstract_instruction<Hardware,EA> {
        int operator() (Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
            ea.env().group(p,ea).receive_donation(p,ea);
            hw.clearLabelStack();
            return 1;
        }
    };
    

} // ea

#endif
