/* neural_network.h
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
#ifndef _ANN_NEURAL_NETWORK_H_
#define _ANN_NEURAL_NETWORK_H_

#include <boost/graph/adjacency_list.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <ann/activation.h>
#include <ann/recurrent.h>
#include <ann/layout.h>

namespace ann {
	
	/*! Neural network.
     
     This is simply a convenience class that wraps a boost::graph, and provides
     some of the common types for a neural network.
	 */
    template <typename Neuron, typename Edge>
    class neural_network 
    : public boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, Neuron, Edge> {
    public:
        typedef boost::adjacency_list<boost::setS, boost::vecS, boost::bidirectionalS, Neuron, Edge> base_type;

        //! Default constructor.
        neural_network() : base_type() {
        }
        
        //! Constructor.
        neural_network(std::size_t n) : base_type(n) {
        }
        
    private:
		friend class boost::serialization::access;
        
		template<class Archive>
		void save(Archive & ar, const unsigned int version) const {
		}
		
		template<class Archive>
		void load(Archive & ar, const unsigned int version) {
		}
		BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

} // ann

#endif
