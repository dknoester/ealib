/* hmm.h
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
#ifndef _FN_HMM_HMM_H_
#define _FN_HMM_HMM_H_

#include <vector>
#include <ea/circular_vector.h>

namespace fn {
	namespace hmm {

		typedef ea::circular_vector<unsigned int> genome;
		
        typedef std::vector<int> state_vector_type;
        
		//! Used to differentiate node types during genome translation.
		enum node_type {
			_FIRST=42, // used only during start_codon detection
			PROBABILISTIC=42, // probabilistic HMM node type
			DETERMINISTIC=43, // deterministic HMM node type
			_LAST=43 // used only during start_codon detection
		};	

		//! Contains limits for various parts of HMM networks.
		struct options {
			static int NODE_INPUT_LIMIT; //!< Limit on the number of inputs.
			static int NODE_OUTPUT_LIMIT; //!< Limit on the number of outputs.
			static int NODE_INPUT_FLOOR; //!< Floor on the number of inputs.
			static int NODE_OUTPUT_FLOOR; //!< Floor on the number of outputs.
			static bool FEEDBACK_LEARNING; //!< Whether feedback learning is enabled.

			//! Calculate the number of inputs based on the given codon.
			static int num_inputs(unsigned char codon);
			
			//! Calculate the number of outputs based on the given codon.
			static int num_outputs(unsigned char codon);		
		};
	} // hmm
} // fn
#endif
