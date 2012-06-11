/* main.cpp
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
#include <algorithm>
#include <numeric>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <boost/archive/text_oarchive.hpp>
#include "globalConst.h"
#include "tAgent.h"
#include "tGame.h"
#include <util/rng.h>
#include <fn/hmm/hmm_network.h>

#define popSize 256

using namespace std;

double replacementRate=0.1;
double perSiteMutationRate=0.01;
int allowedSteps=200;

enum {
	SEED=1,
	NUM_HIDDEN,
	HUNT_REPEATS,
	UPDATES,
	STATS_FILE,
	POP_FILE,
	TRACK_ANCESTRY,
	FAN_LIMIT,
	FAN_FLOOR,
	ARG_COUNT
};

template <typename T>
T parse_arg(const std::string& s) {
	T t;
	std::istringstream iss(s);
	iss >> t;
	return t;
}

int main(int argc, char *argv[]) {
	using namespace fn;
	
	if(argc < ARG_COUNT) {
		cerr << "Usage: " << argv[0] << "<rng seed> <num hidden states> <hunt repeats> <updates> <stats file> <population file> <track ancestry> <fan limit> <fan floor>" << endl;
		exit(-1);
	}
	
	int seed=parse_arg<int>(argv[SEED]);	
	int numHidden=parse_arg<int>(argv[NUM_HIDDEN]);
	int huntRepeats=parse_arg<int>(argv[HUNT_REPEATS]);
	int updates=parse_arg<int>(argv[UPDATES]);
	
	string stats_file=parse_arg<std::string>(argv[STATS_FILE]);
	ostream* stats=0;
	if(stats_file=="-") {
		stats = &cout;
	} else {
		stats = new ofstream(stats_file.c_str());
	}
	
	string pop_file=parse_arg<std::string>(argv[POP_FILE]);
	int track_ancestry=parse_arg<int>(argv[TRACK_ANCESTRY]);
	
	hmm::limits::NODE_INPUT_LIMIT = parse_arg<std::size_t>(argv[FAN_LIMIT]);
	hmm::limits::NODE_OUTPUT_LIMIT = parse_arg<std::size_t>(argv[FAN_LIMIT]);
	hmm::limits::NODE_INPUT_FLOOR = parse_arg<std::size_t>(argv[FAN_FLOOR]);
	hmm::limits::NODE_OUTPUT_FLOOR = parse_arg<std::size_t>(argv[FAN_FLOOR]);
	
	util::default_rng_type rng(seed);
	vector<tAgent*> population;
	tAgent *masterAgent;
	tGame game(&rng);
	
	// initialize the population:
	//
	//
	population.resize(popSize);
	masterAgent=new tAgent(&rng);
	masterAgent->setupRandomAgent(10000);
	masterAgent->setupPhenotype(numHidden);
	
	for(std::size_t i=0;i<population.size();i++){
		population[i]=new tAgent(&rng);
		population[i]->inherit(masterAgent, perSiteMutationRate, 0, track_ancestry);
		population[i]->setupPhenotype(numHidden);
		population[i]->initialize(xDim/4,yDim/4,rng(4));
	}
	masterAgent->retire();
	
	// run the experiment:
	//
	//
	for(int u=1; u<=updates; ++u) {		
		double maxTrialFitness=0.0;
		string maxTrialData;
		double maxGenerationFitness=0.0;
		
		// reset fitnesses:
		for(std::size_t i=0; i<population.size(); ++i) {
			population[i]->fitness = 0.0;
			population[i]->fitnesses.clear();			
		}
		
		// evaluate the entire population huntRepeats number of times, 
		// shuffling the population between each repeat:
		for(int repeat=0; repeat<huntRepeats; ++repeat) {
			std::random_shuffle(population.begin(), population.end(), rng);
			
			// eval every group of 4 agents:
			for(std::size_t i=0; i<population.size(); i+=4){
				tAgent* v[4];
				for(std::size_t j=0;j<4;j++) {
					v[j] = population[i+j];
				}
				
				// play them in the maze:
				string data = game.executeAgentInMaze(v, allowedSteps, false);
				
				// and see how they did:
				// (group fitness, so it's sufficient to check a single agent)
				if(v[0]->fitness > maxTrialFitness) {
					maxTrialFitness = v[0]->fitness;
					maxTrialData = data;
				}
				
				for(std::size_t j=0;j<4;j++) {
					v[j]->fitnesses.push_back(v[j]->fitness);
				}
			}
		}
		
		// the actual fitness of each agent is the mean of all the repeats:
		for(std::size_t i=0; i<population.size(); ++i) {
			population[i]->fitness = std::accumulate(population[i]->fitnesses.begin(), population[i]->fitnesses.end(), 0.0) / population[i]->fitnesses.size();
			maxGenerationFitness = std::max(maxGenerationFitness, population[i]->fitness);
		}			
		
		// output stats about this population:
		//
		//		
		if(u==1) {
			(*stats) << "# 1. update [update]" << endl
			<< "# 2. max trial fitness [maxtrial_fit]" << endl
			<< "# 3. max generation fitness [maxgen_fit]" << endl
			<< "# 4. mean genome size of [genome_size]" << endl
			<< "# 5. mean network size [network_size]" << endl;
		}
		
		if((u==1) || ((u % 100) == 0) || (u==updates)) {
			double network_size=0.0;
			double genome_size=0.0;
			for(std::size_t i=0;i<population.size();i++) {
				network_size += (double)population[i]->network()->num_nodes() / (double)population.size();
				genome_size += (double)population[i]->g.size() / (double)population.size();
			}		
			
			(*stats) << u << " "
			<< maxTrialFitness << " "
			<< maxGenerationFitness << " " 
			<< genome_size << " "
			<< network_size << endl;
		}
		
		// ready the population for the next generation:
		//
		//
		for(std::size_t i=0;i<population.size();i++) {
			// should this agent be replaced?
			if(rng.p(replacementRate) || (population[i]->fitness==0.0)) {
				// yes; preferentially select an agent with high fitness to replace this one:
				std::size_t j;
				do {
					j=rng(population.size());
				} while((j==i) || rng.p(1.0-(pow(1.1,population[j]->fitness)/pow(1.1,maxGenerationFitness))));
				
				population[i]->retire();
				population[i]=new tAgent(&rng);
				population[i]->inherit(population[j], perSiteMutationRate, u, track_ancestry);
				population[i]->setupPhenotype(numHidden);
			}
		}
	}
	
	if(stats != &cout) {
		delete stats;
	}
	
	// dump the final population:
	//
	//
//	ofstream pop_out(pop_file.c_str());
//	boost::archive::text_oarchive pop_archive(pop_out);
//	pop_archive << population;
//	pop_out.close();
	
//	
//	pop_out << "# 1. genome id [id]" << endl
//	<< "# 2. fitness [fitness]" << endl
//	<< "# 3. num inputs [inputs]" << endl
//	<< "# 4. num outputs [outputs]" << endl
//	<< "# 5. num hidden [hidden]" << endl
//	<< "# 6. hex genome [genome]" << endl;
//	
//	for(std::size_t i=0;i<population.size();i++) {
//		tAgent* a=population[i];
//		pop_out << std::dec
//		<< a->ID << " "
//		<< a->fitness << " "
//		<< a->network()->num_inputs() << " "
//		<< a->network()->num_outputs() << " "
//		<< a->network()->num_hidden() << " ";
//		for(std::size_t j=0; j<a->g.size(); ++j) {			
//			pop_out << std::hex << std::setw(4) << std::setfill('0') << a->g[j];
//		}
//		pop_out << endl;
//	}
//	pop_out.close();
	return 0;
}
