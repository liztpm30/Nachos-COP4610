#ifndef PROCMAN_H
#define PROCMAN_H

#include "bitmap.h"
#include "list.h"
#include "pcb.h"
class PCB; 
class Thread;
class AddrSpace;
class Lock; 

class ProcMan {
public: 
	ProcMan();
	~ProcMan(); 
	bool addPCB(PCB *toAdd); 
	bool removePCB(int pid); 
	bool getPCB(int pid); 
	int getProcCount(); 
	void adopt(PCB *parent); 
	void orphanize(); 
private: 
	PCB* procArray[32]; 
	bool occupied[32]; 
	int procCount; 
}



#endif