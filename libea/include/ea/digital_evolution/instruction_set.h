/* digital_evolution/instruction_set.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2014 David B. Knoester, Heather J. Goldsby.
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

#ifndef _EA_DIGITAL_EVOLUTION_INSTRUCTION_SET_H_
#define _EA_DIGITAL_EVOLUTION_INSTRUCTION_SET_H_

#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <limits>
#include <vector>
#include <deque>

#include <ea/mutation.h>

#define DIGEVO_INSTRUCTION_DECL(name) \
template <typename Hardware, typename EA> \
struct name : ealib::instructions::abstract_instruction<Hardware,EA> { \
name(std::size_t cost) : ealib::instructions::abstract_instruction<Hardware,EA>(#name,cost) { } \
virtual ~name() { } \
virtual void operator()(Hardware& hw, typename EA::individual_ptr_type p, EA& ea); }; \
template<typename Hardware,typename EA> void name<Hardware,EA>::operator()(Hardware& hw, typename EA::individual_ptr_type p, EA& ea)


namespace ealib {
    
    /*! This namespace contains all the instructions that are common features of
     digital evolution.  Project specific instructions should NOT go here.
     */
    namespace instructions {
        template <typename Hardware, typename EA>
        struct abstract_instruction {
            //! Constructor.
            abstract_instruction(const std::string& name, std::size_t cost) : _name(name), _cost(cost) {
            }
            
            //! Retrieve the human-readable name of this instruction.
            virtual const std::string& name() { return _name; }
            
            //! Return the cost of this instruction in cycles.
            virtual std::size_t cost(Hardware& hw, typename EA::individual_ptr_type p, EA& ea) {
                return _cost;
            }

            //! Execute this instruction.
            virtual void operator()(Hardware& hw, typename EA::individual_ptr_type p, EA& ea) = 0;
            
            std::string _name; //!< Name of this instruction.
            std::size_t _cost; //!< Cost of executing this instruction.
        };
        
        //! Push nop-a onto the label stack        
        DIGEVO_INSTRUCTION_DECL(nop_a) {
            hw.pushLabelStack(Hardware::NOP_A);
        }
        
        //! Push nop-b onto the label stack
        DIGEVO_INSTRUCTION_DECL(nop_b) {
            hw.pushLabelStack(Hardware::NOP_B);
        }
        
        //! Push nop-c onto the label stack
        DIGEVO_INSTRUCTION_DECL(nop_c) {
            hw.pushLabelStack(Hardware::NOP_C);
        }
        
        //! Spend a cycle doing nothing.
        DIGEVO_INSTRUCTION_DECL(nop_x) {
        }
        
        /*! Allocate memory for this organism's offspring.
         
         Extend the organism's memory by 150%, set the read head to the beginning
         of its memory, and set the write head to the beginning of the newly-
         allocated space.  This instruction only has effect once per lifetime.
         */
        DIGEVO_INSTRUCTION_DECL(h_alloc) {
            hw.extendMemory();
        }
        
        /*! Copy an instruction from the read head to the write head.
         
         The instruction currently pointed to by the read head is copied to the
         current position of the write head.  The write and read heads are then
         each advanced one instruction.
         */
        DIGEVO_INSTRUCTION_DECL(h_copy) {
            typename Hardware::genome_type& r = hw.repr();
            r[hw.getHeadLocation(Hardware::WH)] = r[hw.getHeadLocation(Hardware::RH)];
            hw.advanceHead(Hardware::WH);
            hw.advanceHead(Hardware::RH);
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
                        return;
                    }
                    wh = hw.advance(wh, -1);
                }
            } else {
                hw.advanceHead(Hardware::IP);
            }
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
        }
        
            
        /*! Divide this organism's memory between parent and offspring.
         
         Instructions from the beginning of the organism's memory to the current
         position of the read head are preserved for the parent, while instructions
         between the read head and the write head are split off to form the
         offspring's genome; the offspring is then ``born.''
         */
        DIGEVO_INSTRUCTION_DECL(h_divide) {
            if(hw.age() >= (0.8 * hw.original_size())) {            
                typename Hardware::genome_type& r=hw.repr();
                
                // Check to see if the offspring would be a good length. 
                int divide_pos = hw.getHeadLocation(Hardware::RH);  
                int extra_lines = r.size() - hw.getHeadLocation(Hardware::WH); 
                
                int child_size = r.size() - divide_pos - extra_lines; 
                int parent_size = r.size() - child_size - extra_lines;
                double ratio = 2.0;
                
                if ((child_size < (hw.original_size()/ratio)) || 
                    (child_size > (hw.original_size()*ratio)) || 
                    (parent_size < (hw.original_size()/ratio)) ||
                    (parent_size > (hw.original_size()*ratio))){
                    // fail and die a miserable death!
                    hw.replicated();
                    return;
                }
                
                
                typename Hardware::genome_type::iterator f=r.begin(),l=r.begin();
                std::advance(f, hw.getHeadLocation(Hardware::RH));
                std::advance(l, hw.getHeadLocation(Hardware::WH));                             
                typename Hardware::genome_type offr(f, l);
                
                
                r.resize(parent_size);
                replicate(p, offr, ea);
                hw.replicated();
            }
        }
        
        /*! Reset this organism under the same conditions as h_divide, but do not
         produce offspring.
         */
        DIGEVO_INSTRUCTION_DECL(h_divide_reset_only) {
            if(hw.age() >= (0.8 * hw.original_size())) {
                typename Hardware::genome_type& r=hw.repr();
                
                // Check to see if the offspring would be a good length.
                int divide_pos = hw.getHeadLocation(Hardware::RH);
                int extra_lines = r.size() - hw.getHeadLocation(Hardware::WH);
                
                int child_size = r.size() - divide_pos - extra_lines;
                int parent_size = r.size() - child_size - extra_lines;
                double ratio = 2.0;
                
                if ((child_size < (hw.original_size()/ratio)) ||
                    (child_size > (hw.original_size()*ratio)) ||
                    (parent_size < (hw.original_size()/ratio)) ||
                    (parent_size > (hw.original_size()*ratio))){
                    // fail and die a miserable death!
                    hw.replicated();
                    return;
                }
                
                r.resize(parent_size);
                hw.replicated();
            }
        }
        
        /*! Divide this organism's memory between parent and offspring.
         
         Instructions from the beginning of the organism's memory to the current
         position of the read head are preserved for the parent, while instructions
         between the read head and the write head are split off to form the
         offspring's genome; the offspring is then ``born.''
         
         This instruction provides a soft reset for a multibirth organism.
         */
        DIGEVO_INSTRUCTION_DECL(h_divide_soft_parent_reset) {
            if(hw.age() >= (0.8 * hw.original_size())) {
                typename Hardware::genome_type& r=hw.repr();
                
                // Check to see if the offspring would be a good length.
                int divide_pos = hw.getHeadLocation(Hardware::RH);
                int extra_lines = r.size() - hw.getHeadLocation(Hardware::WH);
                
                int child_size = r.size() - divide_pos - extra_lines;
                int parent_size = r.size() - child_size - extra_lines;
                double ratio = 2.0;
                
                if ((child_size < (hw.original_size()/ratio)) ||
                    (child_size > (hw.original_size()*ratio)) ||
                    (parent_size < (hw.original_size()/ratio)) ||
                    (parent_size > (hw.original_size()*ratio))){
                    // fail!
                    return;
                }
                
                
                typename Hardware::genome_type::iterator f=r.begin(),l=r.begin();
                std::advance(f, hw.getHeadLocation(Hardware::RH));
                std::advance(l, hw.getHeadLocation(Hardware::WH));
                typename Hardware::genome_type offr(f, l);
                
                
                r.resize(parent_size);
                replicate(p, offr, ea);
                hw.replicated_soft_reset();
                
            }
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
                hw.setRegValue(reg, ea.rng()(std::numeric_limits<int>::max()));
                p->inputs().push_front(hw.getRegValue(reg));
            }
        }
        
        /*! Read a new input into ?BX?, where there are only 2 possible inputs
         shared by all organisms.
         */
        DIGEVO_INSTRUCTION_DECL(fixed_input) {
            int reg=hw.modifyRegister();
            
            if(p->inputs().size() == 2) {
                hw.setRegValue(reg, p->inputs().front());
                p->inputs().push_back(p->inputs().front());
                p->inputs().pop_front();
            } else {
                //  0x0f13149f 0x3308e53e 0x556241eb
                // 252908703 856220990 1432502763
                hw.setRegValue(reg, 252908703); 
                p->inputs().push_front(hw.getRegValue(reg));
                p->inputs().push_front(856220990);
            }
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
        }
        
        /*! Reproduce this organism.
         */
        DIGEVO_INSTRUCTION_DECL(repro) {
            if(hw.age() >= (0.8 * hw.original_size())) {            
                replicate(p, hw.repr(), ea);
                hw.replicated();
            }
        }
        
        /*! Store BX nand CX into the ?BX? register.
         For this operation, BX and CX are treated as integers.
         */
        DIGEVO_INSTRUCTION_DECL(nand) {
            int bxVal = hw.getRegValue(Hardware::BX);
            int cxVal = hw.getRegValue(Hardware::CX);
            int nandVal = ~(bxVal & cxVal);
            hw.setRegValue(hw.modifyRegister(), nandVal);
        }
        
        //! Push the value in ?bx? on to the stack.
        DIGEVO_INSTRUCTION_DECL(push) {
            int bxVal = hw.getRegValue(hw.modifyRegister());
            hw.push_stack(bxVal);
        }
        
        //! Pop the value from the stack into ?bx?.
        DIGEVO_INSTRUCTION_DECL(pop) {
            if(!hw.empty_stack()) {
                hw.setRegValue(hw.modifyRegister(), hw.pop_stack());
            }
        }
        
        //! Swap the contents of ?bx? and ?cx?.
        DIGEVO_INSTRUCTION_DECL(swap) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            
            int bxVal = hw.getRegValue(rbx);
            int cxVal = hw.getRegValue(rcx);
            
            hw.setRegValue(rbx, cxVal);
            hw.setRegValue(rcx, bxVal);
        }
        
        //! Latch the data contents of the organism's location.
        DIGEVO_INSTRUCTION_DECL(latch_ldata) {
            int bxVal = hw.getRegValue(hw.modifyRegister());
            if(!exists<LOCATION_DATA>(*p->location())) {
                put<LOCATION_DATA>(bxVal,*p->location());
            }
        }

        //! Set the data contents of the organism's location.
        DIGEVO_INSTRUCTION_DECL(set_ldata) {
            int bxVal = hw.getRegValue(hw.modifyRegister());
            put<LOCATION_DATA>(bxVal,*p->location());
        }

        //! Get the data contents of the organism's location, if it exists.
        DIGEVO_INSTRUCTION_DECL(get_ldata) {
            if(exists<LOCATION_DATA>(*p->location())) {
                hw.setRegValue(hw.modifyRegister(), get<LOCATION_DATA>(*p->location()));
            }
        }

        //! Get the data contents of a neighboring location, if it exists.
        DIGEVO_INSTRUCTION_DECL(get_neighbor_ldata) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p,ea);            
            if(exists<LOCATION_DATA>(l)) {
                hw.setRegValue(hw.modifyRegister(), get<LOCATION_DATA>(l));
            }
        }
        
        //! Get whether a neighboring organism exists.
        DIGEVO_INSTRUCTION_DECL(is_neighbor) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p);
            if(l.occupied()) {
                hw.setRegValue(hw.modifyRegister(), 1);
            } else {
                hw.setRegValue(hw.modifyRegister(), 0);
            }
        }
        

        /*! Sense the state of the enviroment.
         
         This instruction sets a register to a bit vector, where a 1 at position i
         means that, if the individual performs task i, (1) the reaction will be 
         triggered, and (2) that the associated resource's level is greater than
         or equal to the task's limit.
         */
        DIGEVO_INSTRUCTION_DECL(sense_env) {
            typedef typename EA::task_library_type::tasklist_type tasklist_type;
            tasklist_type& tasklist = ea.tasklib().tasks();
            
            int env=0;
            std::size_t j=0;
            for(typename tasklist_type::iterator i=tasklist.begin(); i!=tasklist.end(); ++i, ++j) {
                if((*i)->reaction_occurs(*p,ea)) {
                    if(!(*i)->is_limited() // unlimited resource
                       || ((*i)->consumed_resource()->level() >= (*i)->limit())) { // limited, but sufficient amount
                        env |= 0x01 << j;
                    }
                }
            }

            hw.setRegValue(hw.modifyRegister(), env);
        }

        //! Increment the value in ?bx?.
        DIGEVO_INSTRUCTION_DECL(inc) {
            int rbx = hw.modifyRegister();
            hw.setRegValue(rbx, hw.getRegValue(rbx)+1);                
        }
        
        //! Decrement the value in ?bx?.
        DIGEVO_INSTRUCTION_DECL(dec) {
            int rbx = hw.modifyRegister();
            hw.setRegValue(rbx, hw.getRegValue(rbx)-1);
        }
        
        //! Send a message to the currently-faced neighbor.
        DIGEVO_INSTRUCTION_DECL(tx_msg) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p);
            if(l.occupied()) {
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
            }                
        }
        
        //! Send a message to the currently-faced neighbor.
        DIGEVO_INSTRUCTION_DECL(tx_msg_check_task) {
            typename EA::environment_type::location_type& l=*ea.env().neighbor(p);            
            if(l.occupied()) {
                int rbx = hw.modifyRegister();
                int rcx = hw.nextRegister(rbx);
                l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
            } 
            p->outputs().push_front(hw.getRegValue(hw.modifyRegister()));
            p->outputs().resize(1);
            ea.tasklib().check_tasks(*p,ea);
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
        }
        
        //! Broadcast a message.
        DIGEVO_INSTRUCTION_DECL(bc_msg) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            
            typedef typename EA::environment_type::neighborhood_iterator neighborhood_iterator;
            std::pair<neighborhood_iterator,neighborhood_iterator> ni=ea.env().neighborhood(*p);
            for(; ni.first!=ni.second; ++ni.first) {
                typename EA::environment_type::location_type& l=*ni.first;
                if(l.occupied()) {
                    l.inhabitant()->hw().deposit_message(hw.getRegValue(rbx), hw.getRegValue(rcx));
                }
            }
        }
        
        //! Rotates the organism by ?bx? * pi/4.
        DIGEVO_INSTRUCTION_DECL(rotate) {
            p->position().rotate(hw.getRegValue(hw.modifyRegister()) * M_PI/4);
        }
        
        //! Rotates the organism clockwise by pi/4.
        DIGEVO_INSTRUCTION_DECL(rotate_cw) {
            p->position().rotate_cw();
        }
        
        //! Rotates the organism counter-clockwise by pi/4.
        DIGEVO_INSTRUCTION_DECL(rotate_ccw) {
            p->position().rotate_ccw();
        }
        
        //! Execute the next instruction if ?bx? < ?cx?.
        DIGEVO_INSTRUCTION_DECL(if_less) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            if(hw.getRegValue(rbx) >= hw.getRegValue(rcx)) {
                hw.advanceHead(Hardware::IP);
            }
        }
        
        //! Execute the next instruction if ?bx? == ?cx?.
        DIGEVO_INSTRUCTION_DECL(if_equal) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            if(hw.getRegValue(rbx) != hw.getRegValue(rcx)) {
                hw.advanceHead(Hardware::IP);
            }
        }
        
        //! Execute the next instruction if ?bx? != ?cx?.
        DIGEVO_INSTRUCTION_DECL(if_not_equal) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx);
            if(hw.getRegValue(rbx) == hw.getRegValue(rcx)) {
                hw.advanceHead(Hardware::IP);
            }
        }
        
        //! Get the organism's current position
        DIGEVO_INSTRUCTION_DECL(get_xy) {
            int rbx = hw.modifyRegister();
            int rcx = hw.nextRegister(rbx); 
                        
            hw.setRegValue(rbx,ea.env().location(p->position())->x);
            hw.setRegValue(rcx,ea.env().location(p->position())->y);
        }
        
        //! Get whether a neighboring organism exists.
        DIGEVO_INSTRUCTION_DECL(is_origin) {
            if ((ea.env().location(p->position())->x == 0) &&
                (ea.env().location(p->position())->y == 0)) {
                hw.setRegValue(hw.modifyRegister(), 1);
            } else {
                hw.setRegValue(hw.modifyRegister(), 0);
            }
        }
        
        //! Get the organism's age
        DIGEVO_INSTRUCTION_DECL(get_age){
            int rbx = hw.modifyRegister();
            hw.setRegValue(rbx,hw.age());
        }
        
        //! Jump the organism's IP head; Move head ?IP? by amount in CX register
        DIGEVO_INSTRUCTION_DECL(jump_head){
            int rbx = hw.modifyRegister();
            int jumpAmt = hw.getRegValue(rbx);
            hw.advanceHead(Hardware::IP, jumpAmt);
        }

    } // instructions


    /*! Instruction set architecture for digital evolution.
     */
    template <typename EA>
    class instruction_set {
    public: 
        typedef EA ea_type;
        typedef typename ea_type::hardware_type hardware_type;
        typedef typename ea_type::individual_ptr_type individual_ptr_type;
        
        typedef instructions::abstract_instruction<hardware_type,ea_type> inst_type;
        typedef boost::shared_ptr<inst_type> inst_ptr_type;
        typedef std::vector<inst_ptr_type> isa_type;
        
        typedef std::map<std::string, std::size_t> name_map_type;
        
        //! Constructor.
        instruction_set() {
        }
        
        //! Initialize the ISA.
        void initialize(EA& ea) {
            put<MUTATION_UNIFORM_INT_MIN>(0, ea);
            put<MUTATION_UNIFORM_INT_MAX>(_isa.size(), ea);
        }
        
        //! Append the given instruction to the ISA.
        template <template <typename,typename> class Instruction>
        void append(std::size_t cost) {
            boost::shared_ptr<inst_type> p(new Instruction<hardware_type,ea_type>(cost));
            _isa.push_back(p);
            _name[p->name()] = _isa.size() - 1;
        }
        
        //! Knockout an instruction, replacing it with one of given cost.
        template <template <typename,typename> class Knockout, template <typename,typename> class Replacement>
        void knockout(std::size_t cost) {
            Knockout<hardware_type,ea_type> k(0);
            std::size_t i=operator[](k.name());
            boost::shared_ptr<inst_type> p(new Replacement<hardware_type,ea_type>(cost));
            _isa[i] = p;
        }
        
        //! Execute instruction i.
        void operator()(std::size_t i, hardware_type& hw, individual_ptr_type p, ea_type& ea) {
            (*_isa[i])(hw,p,ea);
        }
        
        //! Retrieve a pointer to instruction i.
        inst_ptr_type operator[](std::size_t i) {
            return _isa[i];
        }
        
        //! Retrieve the index of instruction inst.
        std::size_t operator[](const std::string& inst) {
            name_map_type::iterator i=_name.find(inst);
            if(i ==_name.end()) {
                throw std::invalid_argument("could not find instruction: " + inst + " in the current ISA.");
            }
            
            return i->second;
        }
        
        //! Return the number of instructions available in the ISA.
        std::size_t size() const { return _isa.size(); }
        
    protected:
        isa_type _isa; //!< List of available instructions.
        name_map_type _name; //<! Map of human-readable instruction names to their index in the ISA.
    };        
    
    //! Helper method to add an instruction to the ISA with the given cost.
    template <template <typename,typename> class Instruction, typename EA>
    void append_isa(std::size_t cost, EA& ea) {
        ea.isa().template append<Instruction>(cost);
    }

    //! Helper method to add an instruction to the ISA with a cost of 1.
    template <template <typename,typename> class Instruction, typename EA>
    void append_isa(EA& ea) {
        ea.isa().template append<Instruction>(1);
    }

    //! Helper method to replace an instruction in the ISA with an instruction of given cost.
    template <template <typename,typename> class Knockout, template <typename,typename> class Replacement,  typename EA>
    void knockout(std::size_t cost, EA& ea) {
        ea.isa().template knockout<Knockout,Replacement>(cost);
    }

    //! Helper method to replace an instruction in the ISA with a cost of 1.
    template <template <typename,typename> class Knockout, template <typename,typename> class Replacement,  typename EA>
    void knockout(EA& ea) {
        ea.isa().template knockout<Knockout,Replacement>(1);
    }

} // ea

#endif
