/*
 *  tGame.h
 *  HMMBrain
 *
 *  Created by Arend on 9/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef _tGame_h_included_
#define _tGame_h_included_

#include "globalConst.h"
#include "tAgent.h"
#include <string>
#include <vector>
#include <map>
#include <util/rng.h>

using namespace std;

#define xDim 256
#define yDim 16
#define startMazes 1
#define cPI 3.14159265

class tGame{
public:
	string executeAgentInMaze(tAgent* agent[4],int updates,bool record);
	int cX,cY;
	tGame(util::default_rng_type* rng);
	~tGame();
	
protected:
	util::default_rng_type* _rng;
};
#endif
