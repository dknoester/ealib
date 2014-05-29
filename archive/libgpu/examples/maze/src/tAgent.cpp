/*
 *  tAgent.cpp
 *  HMMBrain
 *
 *  Created by Arend on 9/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include "tAgent.h"

int masterID=0;

tAgent::tAgent() : fitness(0.0), _rng(0) {
}

tAgent::tAgent(util::default_rng_type* rng) : fitness(0.0), _rng(rng) {
	int i;
	nrPointingAtMe=1;
	ancestor=NULL;
	for(i=0;i<maxNodes;i++){
		states[i]=0;
		newStates[i]=0;
	}
	bestSteps=-1;
	ID=masterID;
	masterID++;
	saved=false;
	nrOfOffspring=0;
	totalSteps=0;
	retired=false;
	food=0;
}

tAgent::~tAgent(){
	if(ancestor!=NULL){
		ancestor->nrPointingAtMe--;
		if(ancestor->nrPointingAtMe==0)
			delete ancestor;
	}
}

void tAgent::setupRandomAgent(int nucleotides){
	int i;
	g.resize(nucleotides);
	for(i=0;i<nucleotides;i++)
		g[i]=127;
	ampUpStartCodons();
}

void tAgent::loadAgent(char* filename){
	FILE *f=fopen(filename,"r+t");
	int i;
	g.clear();
	while(!(feof(f))){
		fscanf(f,"%i	",&i);
		g.push_back((unsigned char)(i&255));
	}
}

void tAgent::ampUpStartCodons(void){
	int i,j;
	for(i=0;i<4;i++)
	{
		j=(*_rng)(g.size()-100);
		g[j]=42;
		g[j+1]=(255-42);
		for(int k=2;k<20;k++)
			g[j+k]=(*_rng)(256);
	}
}

void tAgent::inherit(tAgent *from, double mutationRate, int theTime, int track_ancestry) {
	int nucleotides=from->g.size();
	int i,s,o,w;
	double localMutationRate=4.0/from->g.size();
	vector<unsigned char> buffer;
	born=theTime;
	if(track_ancestry) {
		ancestor=from;
		from->nrPointingAtMe++;
		from->nrOfOffspring++;
	}
	g.clear();
	g.resize(from->g.size());
	for(i=0;i<nucleotides;i++) {
		if(_rng->p(localMutationRate)) {
			g[i]=(*_rng)(256);
		}	else {
			g[i]=from->g[i];
		}
	}
	if(_rng->p(0.05) && (g.size()<20000)) {
		//duplication
		w=15+(*_rng)(512);
		s=(*_rng)(g.size()-w);
		o=(*_rng)(g.size());
		buffer.clear();
		buffer.insert(buffer.begin(),g.begin()+s,g.begin()+s+w);
		g.insert(g.begin()+o,buffer.begin(),buffer.end());
	}
	if(_rng->p(0.02) && (g.size()>1000)){
		//deletion
		w=15+(*_rng)(512);
		s=(*_rng)(g.size()-w);
		g.erase(g.begin()+s,g.begin()+s+w);
	}
	fitness=0.0;
}

void tAgent::setupPhenotype(int numHidden){
	_hmm.reset(new hmm::hmm_network(g,8,2,numHidden));
}

void tAgent::retire() {
	retired=true;
	nrPointingAtMe--;
	if(nrPointingAtMe==0) {
		delete this;
	}
}

unsigned char * tAgent::getStatesPointer(void){
	return states;
}

void tAgent::resetBrain(void) {
	bzero(states, sizeof(states));
	bzero(newStates, sizeof(newStates));
	_hmm->clear();
}

void tAgent::updateStates(void) {
	_hmm->update(states, states+8, newStates+maxNodes-2,*_rng);
	memcpy(states, newStates, sizeof(newStates));
	bzero(newStates, sizeof(newStates));
	totalSteps++;
}

void tAgent::showBrain(void){
	for(int i=0;i<maxNodes;i++)
		cout<<(int)states[i];
	cout<<endl;
}

void tAgent::initialize(int x, int y, int d){
	xPos=x;
	yPos=y;
	direction=d;
	steps=0;
}

tAgent* tAgent::findLMRCA(void){
	tAgent *r,*d;
	if(ancestor==NULL)
		return NULL;
	else{
		r=ancestor;
		d=NULL;
		while(r->ancestor!=NULL){
			if(r->ancestor->nrPointingAtMe!=1)
				d=r;
			r=r->ancestor;
		}
		return d;
	}
}

void tAgent::saveFromLMRCAtoNULL(FILE *statsFile,FILE *gFile){
	if(ancestor!=NULL)
		ancestor->saveFromLMRCAtoNULL(statsFile,gFile);
	
	if(!saved){ 
		fprintf(statsFile,"%i	%i	%i	%f	%f	%i	%f\n",ID,born,(int)g.size(),convFitness,fitness,bestSteps,(float)totalSteps/(float)nrOfOffspring);
		fprintf(gFile,"%i	",ID);
		for(std::size_t i=0;i<g.size();i++)
			fprintf(gFile,"	%i",g[i]);
		fprintf(gFile,"\n");
		saved=true;
	}
	if((saved)&&(retired)) g.clear();
}

void tAgent::saveLOD(FILE *statsFile,FILE *gFile){
	if(ancestor!=NULL)
		ancestor->saveLOD(statsFile,gFile);
	fprintf(statsFile,"%i	%i	%i	%f	%f	%i	%f\n",ID,born,(int)g.size(),convFitness,fitness,bestSteps,(float)totalSteps/(float)nrOfOffspring);
	fprintf(gFile,"%i	",ID);
	for(std::size_t i=0;i<g.size();i++)
		fprintf(gFile,"	%i",g[i]);
	fprintf(gFile,"\n");
	
}

void tAgent::showPhenotype(void){
//	for(int i=0;i<hmmus.size();i++)
//		hmmus[i]->show();
//	cout<<"------"<<endl;
}

void tAgent::saveToDot(char *filename){
	FILE *f=fopen(filename,"w+t");
	int i;
	fprintf(f,"digraph brain {\n");
	fprintf(f,"	ranksep=2.0;\n");
	for(i=0;i<6;i++)
		fprintf(f,"	%i [shape=invtriangle,style=filled,color=red];\n",i);
	for(i=6;i<13;i++)
		fprintf(f,"	%i [shape=circle,color=blue];\n",i);
	for(i=13;i<16;i++)
		fprintf(f,"	%i [shape=circle,style=filled,color=green];\n",i);
//	for(i=0;i<hmmus.size();i++){
//	//	fprintf(f,"	{\n");
//		for(j=0;j<hmmus[i]->ins.size();j++){
//			for(k=0;k<hmmus[i]->outs.size();k++)
//				fprintf(f,"	%i	->	%i;\n",hmmus[i]->ins[j],hmmus[i]->outs[k]);
//		}
//	//	fprintf(f,"	}\n");
//	}
	fprintf(f,"	{ rank=same; 0; 1; 2; 3; 4; 5;}\n"); 
	fprintf(f,"	{ rank=same; 6; 7; 8; 9; 10; 11; 12; }\n"); 
	fprintf(f,"	{ rank=same; 13; 14; 15; }\n"); 
	fprintf(f,"}\n");
	fclose(f);
}




