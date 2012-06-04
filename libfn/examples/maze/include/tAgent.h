/*
 *  tAgent.h
 *  HMMBrain
 *
 *  Created by Arend on 9/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _tAgent_h_included_
#define _tAgent_h_included_

#include "globalConst.h"

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <fn/hmm/hmm_network.h>
#include <util/rng.h>
#include <util/circular_vector.h>

using namespace std;
using namespace fn;

extern int masterID;

typedef util::circular_vector<unsigned int> genome;

class tAgent{
public:
	tAgent();
	tAgent(util::default_rng_type* rng);
	~tAgent();
	void setupRandomAgent(int nucleotides);
	void loadAgent(char* filename);
	void setupPhenotype(int numHidden);
	void inherit(tAgent *from, double mutationRate, int theTime, int track_ancestry);
	unsigned char * getStatesPointer(void);
	void updateStates(void);
	void resetBrain(void);
	void ampUpStartCodons(void);
	void showBrain(void);
	void showPhenotype(void);
	void saveToDot(char *filename);
	
	void initialize(int x, int y, int d);
	tAgent* findLMRCA(void);
	void saveFromLMRCAtoNULL(FILE *statsFile,FILE *genomeFile);
	void saveLOD(FILE *statsFile,FILE *genomeFile);
	void retire(void);

	//! Return this agent's HMM.
	inline hmm::hmm_network* network() { return _hmm.get(); }
	
	//! Support archiving tAgents.
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & g & fitness;
	}
	
	genome g;
	tAgent *ancestor;
	unsigned int nrPointingAtMe;
	unsigned char states[maxNodes], newStates[maxNodes];
	double fitness;
	vector<double> fitnesses;
	double convFitness;
	int food;
	
	int xPos,yPos,direction;
	double sX,sY;
	bool foodAlreadyFound;
	int steps,bestSteps,totalSteps;
	int ID,nrOfOffspring;
	bool saved;
	bool retired;
	int born;
	int touches;
	int pushed;
	
protected:
	boost::shared_ptr<hmm::hmm_network> _hmm;
	util::default_rng_type* _rng;
};

#endif
