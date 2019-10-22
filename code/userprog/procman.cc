#include "procman.h"

ProcMan::ProcMan() {
	procCount = 0; 
	for(int i = 0; i < 32; i++) {
		procArray[i] = NULL; 
		occupied[i] = false; 
	}
}

ProcMan::~ProcMan(){
	delete procArray; 
	delete occupied; 
}

bool ProcMan::addPCB(PCB *toAdd){
	for(int i = 0; i < 32; i++) {
		if(!occupied[i]) {
			procCount++; 
			procArray[i] = toAdd; 
			occupied[i] = true; 
			return true; 
		}
	}
	return false; 
}

bool ProcMan::removePCB(int pid) {
	for(int i = 0; i < 32; i++) {
		if(occupied[i] && procArray[i]->getID() == pid) {
			procArray[i] = NULL; 
			occupied[i] = false; 
			procCount--; 
			return true; 
		}
	}
	return false; 
}

PCB* ProcMan::getPCB(int pid) {
	for(int i = 0; i < 32; i++) {
		if(occupied[i] && procArray[i]->getID() == pid) {
			return procArray[i]; 
		}
	}
	return NULL; 
}

int ProcMan::getProcCount() {
	return procCount; 
}

void ProcMan::adopt(PCB *parent) {
	for(int i = 0; i < 32; i++) {
		if(occupied[i]) {
			procArray[i]->setParent(parent); 
		}
	}
}

void ProcMan::orphanize() {
	for(int i = 0; i < 32; i++) {
		if(occupied[i]) {
			procArray[i]->setParent(NULL); 
		}
	}
}