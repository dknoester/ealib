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
#ifndef _ANN_LAYOUT_H
#define _ANN_LAYOUT_H

#include <boost/random.hpp>
#include <ctime>
#include <vector>

namespace ann {
    namespace layout {
        
        /*! Generates a completely-connected graph.
         
         This is typically used with a Concurrent Time Recurrent Neural Network (CTRNN), which has
         been shown to be a universal smooth approximator.
         */
        template <typename Graph>
        void K(Graph& g) {
            for(std::size_t i=0; i<boost::num_vertices(g); ++i) {
                for(std::size_t j=0; j<boost::num_vertices(g); ++j) {
                    if(i!=j) {
                        boost::add_edge(boost::vertex(i,g), boost::vertex(j,g), g);
                    }
                }
            }
        }
        
	} // layout
} //nn

#endif
