/*
 *  tGame.cpp
 *  HMMBrain
 *
 *  Created by Arend on 9/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "tGame.h"
#include "math.h"

int xm[4]={0,1,0,-1};
int ym[4]={-1,0,1,0};
int xm8[8]={0,1,1,1,0,-1,-1,-1};
int ym8[8]={-1,-1,0,1,1,1,0,-1};

tGame::tGame(util::default_rng_type* rng) : _rng(rng) {
}

tGame::~tGame(){
}

string tGame::executeAgentInMaze(tAgent* agent[4],int updates,bool record){
	int i,j,update,action,inFront,liFront,reFront;
	int totalBarrels;
	unsigned char area[xDimArea][yDimArea];
	tAgent* who[xDimArea][yDimArea];
	string data;

	for(i=0;i<4;i++)
		agent[i]->resetBrain();
	
	for(i=0;i<xDimArea;i++)
		for(j=0;j<yDimArea;j++){
			area[i][j]=0;
			who[i][j]=NULL;
		}
	for(i=0;i<xDimArea;i++){
		area[i][0]=3;
		area[i][yDimArea-1]=3;
	}
	for(j=0;j<yDimArea;j++){
		area[0][j]=3;
		area[xDimArea-1][j]=3;
	}
	area[7][7]=3; area[7][8]=3;
	area[8][7]=3; area[8][8]=3;
	area[5][6]=2; area[10][6]=2;
	area[5][7]=2; area[10][7]=2;
	area[5][8]=2; area[10][8]=2;
	area[5][9]=2; area[10][9]=2;
	area[6][5]=2; area[7][5]=2; 
	area[8][5]=2; area[9][5]=2;
	area[6][10]=2; area[7][10]=2;
	area[8][10]=2; area[9][10]=2;

	agent[0]->xPos=1; agent[0]->yPos=1; agent[0]->direction=((*_rng)(4));
	agent[1]->xPos=xDimArea-2; agent[1]->yPos=1; agent[1]->direction=((*_rng)(4));
	agent[2]->xPos=1; agent[2]->yPos=yDimArea-2; agent[2]->direction=((*_rng)(4));
	agent[3]->xPos=xDimArea-2; agent[3]->yPos=yDimArea-2; agent[3]->direction=((*_rng)(4));
	
	for(i=0;i<4;i++){
		area[agent[i]->xPos][agent[i]->yPos]=1;
		who[agent[i]->xPos][agent[i]->yPos]=agent[i];
	}

	for(update=0;update<updates;update++){
		for(i=0;i<4;i++){
			area[agent[i]->xPos][agent[i]->yPos]=0;
			who[agent[i]->xPos][agent[i]->yPos]=NULL;
			
			liFront=area[agent[i]->xPos+xm8[((agent[i]->direction<<1)-1)&7]][agent[i]->yPos+ym8[((agent[i]->direction<<1)-1)&7]];
			inFront=area[agent[i]->xPos+xm8[(agent[i]->direction<<1)&7]][agent[i]->yPos+ym8[(agent[i]->direction<<1)&7]];
			reFront=area[agent[i]->xPos+xm8[((agent[i]->direction<<1)+1)&7]][agent[i]->yPos+ym8[((agent[i]->direction<<1)+1)&7]];

			agent[i]->states[0]=liFront>>1;
			agent[i]->states[1]=liFront&1;
			agent[i]->states[2]=inFront>>1;
			agent[i]->states[3]=inFront&1;
			agent[i]->states[4]=reFront>>1;
			agent[i]->states[5]=reFront&1;
			agent[i]->states[6]=agent[i]->direction>>1;
			agent[i]->states[7]=agent[i]->direction&1;
			agent[i]->updateStates();

			action=agent[i]->states[maxNodes-2]+(agent[i]->states[maxNodes-1]<<1);
			switch(action){
				case 0: break;
				case 1: 
					agent[i]->direction = (agent[i]->direction+1)&3; 
					break;
				case 2: 
					agent[i]->direction = (agent[i]->direction-1)&3; 
					break;
				case 3: {
					switch(inFront){
						case 0: //empty
							agent[i]->xPos+=xm[agent[i]->direction];
							agent[i]->yPos+=ym[agent[i]->direction];
							break;
						case 1: //other agent
							who[agent[i]->xPos+xm[agent[i]->direction]][agent[i]->yPos+ym[agent[i]->direction]]->pushed=1;
							break;
						case 2: //barrel
							if(area[agent[i]->xPos+(2*xm[agent[i]->direction])][agent[i]->yPos+(2*ym[agent[i]->direction])]==0) {
								area[agent[i]->xPos+(2*xm[agent[i]->direction])][agent[i]->yPos+(2*ym[agent[i]->direction])]=2;
								agent[i]->xPos += xm[agent[i]->direction];
								agent[i]->yPos += ym[agent[i]->direction];
								agent[i]->touches++;
							}	else {
								agent[i]->pushed=1;
							}
							break;
					}
					break;
				}
			}
			
			assert((agent[i]->xPos>=0) && (agent[i]->xPos<xDim));
			assert((agent[i]->yPos>=0) && (agent[i]->yPos<yDim));

			area[agent[i]->xPos][agent[i]->yPos]=1;
			who[agent[i]->xPos][agent[i]->yPos]=agent[i];
		}

		if(record){
			for(i=0;i<4;i++){
				data.push_back('A'+(char)agent[i]->xPos);
				data.push_back('A'+(char)agent[i]->yPos);
				data.push_back('A'+(char)agent[i]->direction);
			}
		}
	}
	
	totalBarrels=0;
	for(i=6;i<10;i++)
		for(j=6;j<10;j++)
			if(area[i][j]==2)
				totalBarrels++;
	
	for(i=0;i<4;i++)
		agent[i]->fitness=(double)totalBarrels;

	return data;
}
