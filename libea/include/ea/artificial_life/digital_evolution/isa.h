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

#ifndef _EA_ARTIFICIAL_LIFE_DIGITAL_EVOLUTION_ISA_H_
#define _EA_ARTIFICIAL_LIFE_DIGITAL_EVOLUTION_ISA_H_

#include <boost/shared_ptr.hpp>
#include <vector>

#define DIGEVO_INSTRUCTION_DECL(name) \
template <typename Hardware, typename EA> \
struct name : ea::instructions::abstract_instruction<Hardware,EA> { \
name() : ea::instructions::abstract_instruction<Hardware,EA>(#name) { } \
virtual int operator()(Hardware& hw, typename EA::individual_ptr_type p, EA& ea); }; \
template<typename Hardware,typename EA> int name<Hardware,EA>::operator()(Hardware& hw, typename EA::individual_ptr_type p, EA& ea)


namespace ea {
    
    /*! This namespace contains all the instructions that are common features of
     digital evolution.  Project specific instructions should NOT go here.
     */
    namespace instructions {
        template <typename Hardware, typename EA>
        struct abstract_instruction {
            //! Constructor.
            abstract_instruction(const std::string& name) : _name(name) {
            }
            
            //! Retrieve the human-readable name of this instruction.
            virtual const std::string& name() { return _name; }
            
            //! Execute this instruction, returning its cost in number of cycles.
            virtual int operator()(Hardware& hw, typename EA::individual_ptr_type p, EA& ea) = 0;
            
            std::string _name; //!< Name of this instruction.
        };
        
        //! Push nop-a onto the label stack        
        DIGEVO_INSTRUCTION_DECL(nop_a) {
            hw.pushLabelStack(Hardware::NOP_A);
            return 0;
        }
        
        //! Push nop-b onto the label stack
        DIGEVO_INSTRUCTION_DECL(nop_b) {
            hw.pushLabelStack(Hardware::NOP_B);
            return 0;
        }
        
        //! Push nop-c onto the label stack
        DIGEVO_INSTRUCTION_DECL(nop_c) {
            hw.pushLabelStack(Hardware::NOP_C);
            return 0;
        }
        
        //! Do nothing.
        DIGEVO_INSTRUCTION_DECL(nop_x) {
            return 1;
        }
        
        /*! Allocate memory for this organism's offspring.
         
         Extend the organism's memory by 150%, set the read head to the beginning
         of its memory, and set the write head to the beginning of the newly-
         allocated space.  This instruction only has effect once per lifetime.
         */
        DIGEVO_INSTRUCTION_DECL(h_alloc) {
            hw.extendMemory();
            return 1;
        }
        
        /*! Copy an instruction from the read head to the write head.
         
         The instruction currently pointed to by the read head is copied to the
         current position of the write head.  The write and read heads are then
         each advanced one instruction.
         */
        DIGEVO_INSTRUCTION_DECL(h_copy) {
            typename Hardware::representation_type& r = hw.repr();
            r[hw.getHeadLocation(Hardware::WH)] = r[hw.getHeadLocation(Hardware::RH)];
            hw.advanceHead(Hardware::WH);
            hw.advanceHead(Hardware::RH);
            return 1;
        }
        
        //! Move the ?IP? head to the same position as the flow control head
        DIGEVO_INSTRUCTION_DECL(mov_head) {
            int h = hw.modifyHead();
            hw.setHeadLocation(h, hw.getHeadLocation(Hardware::FH));
            
            // If we moved the IP, we have to back up one instruction  
            // because of the auto-increment.
            if (h == Hardware::IP) {
                hw.advanceHead(h, -1);
            }
            return 1;
        }
        
        /*! Execute the next instruction if the complement was just copied.
         If there is no preceding label, or the complementary label was not
         just copied, skip the next instruction.
         */
        DIGEVO_INSTRUCTION_DECL(if_label) {
            if(!hw.isLabelStackEmpty()) {
                // what immediately preceeds the write head...
                int wh = hw.advance(hw.getHeadLocation(Hardware::WH), -1);
                std::deque<int> label_comp = hw.getLabelComplement();
                // check through label in reverse order...
                // most recent label is on the back...
                for(int i=(label_comp.size() - 1);  i>=0; --i) { 
                    if(label_comp[i] != static_cast<int>(hw.repr()[wh])) {
                        hw.advanceHead(Hardware::IP);
                        
                        return 1;
                    }
                    wh = hw.advance(wh, -1);
                }
            } else {
                hw.advanceHead(Hardware::IP);
            }
            return 1;
        }
        
        /*! Search forward until the first complement to the label is found.
         
         From the current IP, scan forward in the memory (wrapping around as
         needed) looking for the first complement to the label.  If a complement
         is found, BX is set to the distance to the complement, CX is set to
         its size, and the flow head is set to the instruction immediately 
         following the complement.  If no label is found, BX and CX are set to
         zero, and the flow head is moved to the instruction following h_search.
         */
        DIGEVO_INSTRUCTION_DECL(h_search) {
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
            
            return 1;
        }
        
        /*! Divide this organism's memory between parent and offspring.
         
         Instructions from the beginning of the organism's memory to the current
         position of the read head are preserved for the parent, while instructions
         between the read head and the write head are split off to form the
         offspring's genome; the offspring is then ``born.''
         */
        DIGEVO_INSTRUCTION_DECL(h_divide) {
            if(hw.age() >= (0.8 * hw.original_size())) {            
                typename Hardware::representation_type& r=hw.repr();
                
                typename Hardware::representation_type::iterator f=r.begin(),l=r.begin();
                std::advance(f, hw.getHeadLocation(Hardware::RH));
                int wh_advance = hw.getHeadLocation(Hardware::WH); 
                if (wh_advance <= hw.getHeadLocation(Hardware::RH)) {
                    wh_advance += r.size();
                }
                std::advance(l, wh_advance);                
                
                typename Hardware::representation_type offr(f, l);
                r.resize(hw.original_size());
                
                // This clause prevents crazily sized offspring from 
                // entering the population
                if ((offr.size() > (hw.original_size()/2)) && 
                    (offr.size() < (hw.original_size()*2))) {
                    replicate(p, offr, ea);
                }
                hw.replicated();
            }
            
            return 1;
        }
        
        /*! Read a new input into ?BX?.
         */
        DIGEVO_INSTRUCTION_DECL(input) {
            int reg=hw.modifyRegister();
            
            if(p->inputs().size() == 2) {
                hw.setRegValue(reg, p->inputs().front());
                p->inputs().push_back(p->inputs().front());
                p->inputs().pop_front();
            } else {
                hw.setRegValue(reg, ea.env().read(*p, ea));
                p->inputs().push_front(hw.getRegValue(reg));
            }
            
            return 1;
        }
        
        /*! Output ?BX?.
         
         Executing this instruction triggers task evaluation on this output value
         and the last two input values.  Regardless of the specific cataylst type,
         fitness changes from multiple tasks are multiplied together.
         */
        DIGEVO_INSTRUCTION_DECL(output) {
            p->outputs().push_front(hw.getRegValue(hw.modifyRegister()));
            p->outputs().resize(1);
            ea.tasklib().check_tasks(*p,ea);
            return 1;
        }
        
        /*! Reproduce this organism.
         */
        DIGEVO_INSTRUCTION_DECL(repro) {
            if(hw.age() >= (0.8 * hw.original_size())) {            
                replicate(p, hw.repr(), ea);
                hw.replicated();
            }
            
            return 1;
        }
        
        /*! Store BX nand CX into the ?BX? register.
         For this operation, BX and CX are treated as integers.
         */
        DIGEVO_INSTRUCTION_DECL(nand) {
            int bxVal = hw.getRegValue(Hardware::BX);
            int cxVal = hw.getRegValue(Hardware::CX);
            int nandVal = ~(bxVal & cxVal);
            hw.setRegValue(hw.modifyRegister(), nandVal);
            
            return 1;
        }
        
        //! Push the value in ?bx? on to the stack.
        DIGEVO_INSTRUCTION_DECL(push) {
            int bxVal = hw.getRegValue(hw.modifyRegister());
            hw.push_stack(bxVal);
            
            return 1;
        }
        
        //! Pop the value from the stack into ?bx?.
        DIGEVO_INSTRUCTION_DECL(pop) {
            if(!hw.empty_stack()) {
                hw.setRegValue(hw.modifyRegister(), hw.pop_stack());
            }
            return 1;
        }
        
        //! Swap the contents of ?bx? and ?cx?.
        DIGEVO_INSTRUCTION_DECL(swap) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            
            int bxVal = hw.getRegValue(rbx);
            int cxVal = hw.getRegValue(rcx);
            
            hw.setRegValue(rbx, cxVal);
            hw.setRegValue(rcx, bxVal);
            
            return 1;
        }
        
        /*! Set the data contents of the organism's location.
         */
        DIGEVO_INSTRUCTION_DECL(latch_ldata) {
            int bxVal = hw.getRegValue(hw.modifyRegister());
            if(!exists<LOCATION_DATA>(*p->location())) {
                put<LOCATION_DATA>(bxVal,*p->location());
            }
            return 1;
        }
        
        //! Increment the value in ?bx?.
        DIGEVO_INSTRUCTION_DECL(inc) {
            int rbx = hw.modifyRegister();
            hw.setRegValue(rbx, hw.getRegValue(rbx)+1);                
            return 1;
        }
        
        //! Decrement the value in ?bx?.
        DIGEVO_INSTRUCTION_DECL(dec) {
            int rbx = hw.modifyRegister();
            hw.setRegValue(rbx, hw.getRegValue(rbx)-1);
            return 1;
        }
        
        //! Send a message to the currently-faced neighbor.
        DIGEVO_INSTRUCTION_DECL(tx_msg) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p,ea);            
            if(l.occupied()) {
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
            }                
            return 1;
        }
        
        //! Retrieve a message from the caller's message buffer.
        DIGEVO_INSTRUCTION_DECL(rx_msg) {
            if(hw.msgs_queued()) {
                std::pair<int,int> msg = hw.pop_msg();
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                hw.setRegValue(rbx,msg.first);
                hw.setRegValue(rcx,msg.second);
            }
            
            return 1;
        }
        
        /*! Broadcast a message.
         */
        DIGEVO_INSTRUCTION_DECL(bc_msg) {
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
            
            return 1;
        }
        
        //! Rotate the organism to the heading in ?bx?.
        DIGEVO_INSTRUCTION_DECL(rotate) {
            p->location()->set_heading(hw.getRegValue(hw.modifyRegister()));
            return 1;
        }
        
        //! Rotate the organism clockwise once.
        DIGEVO_INSTRUCTION_DECL(rotate_cw) {
            p->location()->alter_heading(-1);
            return 1;
        }
        
        //! Rotate the organism counter-clockwise once.
        DIGEVO_INSTRUCTION_DECL(rotate_ccw) {
            p->location()->alter_heading(1);
            return 1;
        }
        
        //! Execute the next instruction if ?bx? < ?cx?.
        DIGEVO_INSTRUCTION_DECL(if_less) {
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                if(hw.getRegValue(rbx) >= hw.getRegValue(rcx)) {
                    hw.advanceHead(Hardware::IP);
                }
                
                return 1;
            }
        
        //! Donate any accumulated resource to this organism's group.
        DIGEVO_INSTRUCTION_DECL(donate_group) {
            ea.env().group(p,ea).receive_donation(p,ea);
            return 1;
        }
    } // instructions
    
    
    /*! Instruction set architecture for digital evolution.
     */
    template <typename EA>
    class isa {
    public: 
        typedef EA ea_type;
        typedef typename ea_type::hardware_type hardware_type;
        typedef typename ea_type::individual_ptr_type individual_ptr_type;
        
        typedef instructions::abstract_instruction<hardware_type,ea_type> inst_type;
        typedef std::vector<boost::shared_ptr<inst_type> > isa_type;
        
        typedef std::map<std::string, std::size_t> name_map_type;
        
        //! Constructor.
        isa() {
        }
        
        template <template <typename,typename> class Instruction>
        void append() {
            boost::shared_ptr<inst_type> p(new Instruction<hardware_type,ea_type>());
            _isa.push_back(p);
            _name[p->name()] = _isa.size() - 1;
        }            
        
        bool operator()(std::size_t inst, hardware_type& hw, individual_ptr_type p, ea_type& ea) {
            return (*_isa[inst])(hw,p,ea);
        }
        
        bool is_nop(std::size_t inst) {
            return _isa[inst]->is_nop();
        }
        
    protected:
        isa_type _isa; //!< List of available instructions.
        name_map_type _name; //<! Map of human-readable instruction names to their index in the ISA.
    };        
    
    template <template <typename,typename> class Instruction, typename EA>
    void append_isa(EA& ea) {
        ea.isa().template append<Instruction>();
    }
    
} // ea

#endif
