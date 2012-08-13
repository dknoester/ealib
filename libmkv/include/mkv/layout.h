/* layout.h
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester.
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
#ifndef _MKV_LAYOUT_H_
#define _MKV_LAYOUT_H_

#include <mkv/markov_network.h>
#include <mkv/detail/prob_node.h>

namespace mkv {
    
    /*! Layout an x-by-y-by-z markov network.
     
     Z nodes of size x-by-y, no hidden states.
     */
    void layout_uniform(markov_network& net, std::size_t x, std::size_t y, std::size_t z) {
        using namespace mkv::detail;
        markov_network fnet(x*z, y*z, 0, rng);
        
        std::vector<int> uniform((1<<x)*(1<<y), 1);
        index_list_type inputs(x), outputs(y);
        
        std::size_t offset=0;
        for(std::size_t i=0; i<z; ++i) {
            for(std::size_t j=0; j<x; ++j) {
                inputs[j] = j+offset;
            }
            
                inputs[1] = 
                outputs[0] = 8;
            markov_network::nodeptr_type p(new probabilistic_history_mkv_node(1, inputs, outputs, unit.begin(), get<NODE_ALLOW_ZERO>(ea)));
            fnet.append(p);
        }
        {
            inputs[0] = 1;
            inputs[1] = 5;
            outputs[0] = 9;
            markov_network::nodeptr_type p(new probabilistic_history_mkv_node(1, inputs, outputs, unit.begin(), get<NODE_ALLOW_ZERO>(ea)));
            fnet.append(p);
        }
        {
            inputs[0] = 2;
            inputs[1] = 6;
            outputs[0] = 10;
            markov_network::nodeptr_type p(new probabilistic_history_mkv_node(1, inputs, outputs, unit.begin(), get<NODE_ALLOW_ZERO>(ea)));
            fnet.append(p);
        }
        {
            inputs[0] = 3;
            inputs[1] = 7;
            outputs[0] = 11;
            markov_network::nodeptr_type p(new probabilistic_history_mkv_node(1, inputs, outputs, unit.begin(), get<NODE_ALLOW_ZERO>(ea)));
            fnet.append(p);
        }
   
    }
    
    
    //! Base class for markov network instruments.
    class mkv_instrument {
    public:
        mkv_instrument() { }
        virtual ~mkv_instrument() { }
        virtual void top_half(markov_network& mkv) { }
        virtual void bottom_half(markov_network& mkv) { }
        virtual void clear() { }
        virtual std::ostream& operator<<(std::ostream& out) { return out; }
    };
    
    namespace instruments {
        
        struct pin_state : mkv_instrument {
            pin_state(std::size_t s, int v) : _state(s), _value(v) {
            }
            
            virtual void top_half(markov_network& mkv) { 
                mkv.svm().state_tminus1(_state) = _value;
            }
            
            std::size_t _state;
            int _value;
        };
        
    }
    
} // mkv

#endif
