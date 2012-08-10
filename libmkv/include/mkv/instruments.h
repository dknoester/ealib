/* instruments.h
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
#ifndef _MKV_INSTRUMENTS_H_
#define _MKV_INSTRUMENTS_H_

#include <mkv/markov_network.h>

namespace mkv {
    
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
