/* hmm.cpp
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
#include <fn/hmm/hmm.h>

int fn::hmm::options::NODE_INPUT_LIMIT=8;
int fn::hmm::options::NODE_OUTPUT_LIMIT=8;
int fn::hmm::options::NODE_INPUT_FLOOR=1;
int fn::hmm::options::NODE_OUTPUT_FLOOR=1;
bool fn::hmm::options::FEEDBACK_LEARNING=false;

/*! Calculate the number of inputs based on the given codon.
 
 Given the default values for the options and floors, what we're doing here is
 constraining the range of the number of inputs and outputs.  Effectively, the genome
 contains the number of inputs in *excess* of the floor, capped by the limit.
 
 Let codon c==10:
 c % (8-1) + 1 == 10%7+1 == 4
 
 Let codon c==8:
 8%7+1==2 
 
 Let codon c==2:
 2%7+1==3
 */
int fn::hmm::options::num_inputs(unsigned char codon) {
	int range=NODE_INPUT_LIMIT-NODE_INPUT_FLOOR;
	if(range==0) {
		return NODE_INPUT_LIMIT;
	} else {
		return (codon % range) + NODE_INPUT_FLOOR;
	}
}

/*! Calculate the number of outputs based on the given codon.
 
 See above for calculation number of inputs and outputs.
 */
int fn::hmm::options::num_outputs(unsigned char codon) {
	int range=NODE_OUTPUT_LIMIT-NODE_OUTPUT_FLOOR;
	if(range == 0) {
		return NODE_OUTPUT_LIMIT;
	} else {
		return (codon % range) + NODE_OUTPUT_FLOOR;
	}
}
