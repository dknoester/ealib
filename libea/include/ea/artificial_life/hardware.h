/* artificial_life/hardware.h 
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

#ifndef _EA_ARTIFICIAL_LIFE_HARDWARE_H_
#define _EA_HARDWARE_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/deque.hpp>
#include <deque>
#include <strings.h>

#include <ea/representations/circular_genome.h>
#include <ea/artificial_life/instructions.h>

namespace ea {
    
    /*! Avida Basic CPU hardware.
     
     <more goes here>
     */
    class hardware {
    public:            
        typedef circular_genome<unsigned int> representation_type;
        
        const static int NOP_A = 0; 
        const static int NOP_B = 1; 
        const static int NOP_C = 2;
        const static int NOP_X = 3;
        
        const static int NUM_HEADS = 4;
        const static int NUM_REGISTERS = 3;
        
        const static int IP = 0;
        const static int RH = 1;
        const static int WH = 2; 
        const static int FH = 3; 
        
        const static int AX = 0; 
        const static int BX = 1; 
        const static int CX = 2;
        
        //! Constructor.
        hardware() {
            initialize();
        }
        
        //! Constructor.
        hardware(const representation_type& repr) : _repr(repr) {
            initialize();
        }
        
        //! Destructor.
        ~hardware() {
        }
        
        //! Returns true if hardware(s) are equivalent.
        bool operator==(const hardware& that) {
            return (_repr == that._repr)
            && std::equal(_head_position, _head_position+NUM_HEADS, that._head_position)
            && std::equal(_regfile, _regfile+NUM_REGISTERS, that._regfile)
            && (_label_stack == that._label_stack)
            && (_age == that._age)
            && (_mem_extended == that._mem_extended)
            && (_stack == that._stack)
            && (_msgs == that._msgs);
        }
        
        //! (Re-) Initialize this hardware.
        void initialize() {
            bzero(_head_position, sizeof(int)*NUM_HEADS);
            bzero(_regfile, sizeof(int)*NUM_REGISTERS);
            _age = 0;
            _mem_extended = false;
            _label_stack.clear();
            _orig_size = _repr.size();
            _stack.clear();
            _msgs.clear();
        }
        
        /*! Step this hardware by n virtual CPU cycles.
         */
        template <typename AL>
        void execute(std::size_t n, typename AL::individual_ptr_type p, AL& al) {
            
            typename AL::isa_type& isa=al.isa();
            int cycle_cost = 0;
            while (n > 0) {
                int cur_inst = _repr[_head_position[IP]];
                cycle_cost = isa(cur_inst, *this, p, al);
                n-=cycle_cost;
                _age+=cycle_cost;
                if(cycle_cost > 0) {
                    clearLabelStack();
                }
                advanceHead(IP);
            }
        }
        
        void replicated() {
            initialize();
            advanceHead(IP, -1);
            --_age;
        }
        
        int age() { return _age; }
        
        //! Get the register to be modified
        int modifyRegister() { 
            if (_label_stack.size() == 0) {
                return BX;
            } else {
                return popLabelStack();
            }
        }
        
        //! Get the next register given x.
        int nextRegister(int x) {
            return (x+1) % NUM_REGISTERS;
        }
        
        //! Get a register value
        int getRegValue(int pos){
            assert(pos < NUM_REGISTERS);
            return _regfile[pos];
        }
        
        //! Set a register value
        void setRegValue(int pos, int val) {
            assert(pos < NUM_REGISTERS);
            _regfile[pos] = val;
        }
        
        //! Push a label on the label stack
        void pushLabelStack(int label) { 
            _label_stack.push_back(label);
        }
        
        //! Pop one label off the label stack
        int popLabelStack()  { 
            int label = _label_stack.front(); 
            _label_stack.pop_front();
            return label;
        }
        
        //! Clear the label stack
        void clearLabelStack() { _label_stack.clear(); }
        
        //! Check if the label stack is empty
        bool isLabelStackEmpty() { return _label_stack.empty(); }
        
        //! Get the head to be modified
        int modifyHead() { 
            if (_label_stack.size() == 0) 
                return IP;
            else 
                return popLabelStack();
        }
        
        //! Set the location of head h to position pos
        void setHeadLocation(int h, int pos){
            assert (h < NUM_HEADS); 
            assert (pos < _repr.size());
            _head_position[h] = pos;
        } 
        
        //! Get the location of a head
        int getHeadLocation(int h) {
            assert (h < NUM_HEADS); 
            return (_head_position[h]);
        }
        
        //! Advance a head by x positions
        void advanceHead (int h, int x=1) {
            assert (h < NUM_HEADS); 
            _head_position[h] = advance(_head_position[h], x);
        }
        
        //! Advance the head and return the new position.
        int advance (int hp, int x=1)  {
            int pos = hp + x;
            if(pos < 0) {
                pos = _repr.size() + pos;
            } 
            pos %= _repr.size();
            return pos;
        }
        
        
        //! Get a label complement
        std::deque<int> getLabelComplement() {
            std::deque<int> comp; 
            
            for(std::size_t i=0; i<_label_stack.size(); ++i) {
                int comp_label = (_label_stack[i] + 1) % NUM_REGISTERS; 
                comp.push_back(comp_label);
            }
            return comp;
        }
        
        /*! Search forward in memory from the IP for label.
         
         If the label is found, return the distance to it from the IP,
         otherwise return -1 */
        int findLabel(std::deque<int> label) {
            if (label.size() > 0) {
                int d = 0; 
                int i = _head_position[IP];
                int exited;
                while(true) { 
                    exited = true;
                    for(std::size_t j=0; j<label.size(); ++j) {
                        int k = advance(i, j); 
                        if(static_cast<int>(_repr[k]) != label[j]) { 
                            exited = false;
                            break; 
                        } 
                    }
                    if (exited) {
                        return d; 
                    }
                    i = advance(i); 
                    d += 1; 
                    
                    // if we've looped...
                    if (i == _head_position[IP]) break;
                }
            }
            return -1; 
        }
        
        /*! Search forward in memory from the IP for the complement of a label.
         
         If the label complement is found, return the distance to it from the IP,
         and the complement size. */
        std::pair<int, int> findComplementLabel() {
            std::pair <int, int> retVal; 
            if (_label_stack.size() > 0) {
                std::deque<int> comp = getLabelComplement();
                int dist = findLabel(comp);
                retVal = std::make_pair(dist, comp.size());
            } else {
                retVal = std::make_pair(-1, 0);
            }
            return retVal;
        }
        
        /*! Allocate memory for this organism's offspring.
         
         Extend the organism's memory by 150%, set the read head to the beginning
         of its memory, and set the write head to the beginning of the newly-
         allocated space.  This instruction only has effect once per lifetime.
         */
        void extendMemory() { 
            if(!_mem_extended) {
                _mem_extended = true;
                _repr.resize(static_cast<std::size_t>(_orig_size * 2.5), NOP_X);
//                setHeadLocation(RH, 0);
//                setHeadLocation(WH, _orig_size);
            }
        }
        
        //! Retrieve this hardware's representation (its genome).
        representation_type& repr() { return _repr; }
        
        void push_stack(int x) { _stack.push_front(x); while(_stack.size() > 10) { _stack.pop_back(); } }
        bool empty_stack() { return _stack.empty(); }
        int pop_stack() { int x = _stack.front(); _stack.pop_front(); return x; }
        
        void deposit_message(int label, int data) {
            if(_msgs.size() < 10) {
                _msgs.push_back(std::make_pair(label,data));
            }
        };
        
        std::size_t msgs_queued() { return _msgs.size(); }
        
        std::pair<int,int> pop_msg() { std::pair<int,int> msg=_msgs.front(); _msgs.pop_front(); return msg; }
        
        std::size_t original_size() { return _orig_size; }
    protected:
        representation_type _repr; //!< This hardware's "program".
        int _head_position[NUM_HEADS]; //!< Positions of the various heads.
        int _regfile[NUM_REGISTERS]; //!< ...
        
        std::deque<int> _label_stack;
        int _age;
        bool _mem_extended; 
        std::size_t _orig_size;
        std::deque<int> _stack;
        std::deque<std::pair<int,int> > _msgs;            
        
    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("representation", _repr);
            ar & boost::serialization::make_nvp("head_positions", _head_position);
            ar & boost::serialization::make_nvp("register_file", _regfile);
            ar & boost::serialization::make_nvp("labels", _label_stack);
            ar & boost::serialization::make_nvp("age", _age);
            ar & boost::serialization::make_nvp("extended", _mem_extended);
            ar & boost::serialization::make_nvp("original_size", _orig_size);
            ar & boost::serialization::make_nvp("stack", _stack);
            ar & boost::serialization::make_nvp("messages", _msgs);
        }
    };
    
} // ea

#endif
