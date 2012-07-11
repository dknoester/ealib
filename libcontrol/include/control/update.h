/* update.h
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
#ifndef _FN_UPDATE_H_
#define _FN_UPDATE_H_

#include <stdexcept>

namespace control {
    
    /*! Update a functional network n times.
     */
    template <typename Network, typename InputIterator, typename OutputIterator>
    void update_n(std::size_t n, Network& net, InputIterator f, InputIterator l, OutputIterator result) {
        if(static_cast<std::size_t>(std::distance(f,l)) != net.input_size()) {
            throw std::invalid_argument("input_size of network not equal to number of inputs provided");
        }
        
        for( ; n>0; --n) {
            net.rotate();
            std::copy(f, l, net.tminus1_inputs());
            
            net.top_half();
            for(typename Network::nodelist_type::iterator i=net.begin(); i!=net.end(); ++i) {
                (*i)->update(net);
            }
            net.bottom_half();
        }
        
        std::copy(net.t_outputs(), net.t_outputs()+net.output_size(), result);
    }
    
} // fn

#endif
