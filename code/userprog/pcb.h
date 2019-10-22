#ifndef PCB_H
#define PCB_H


#include "synch.h"
#include "thread.h"
#include "list.h"
#include "procman.h"

class Thread;
class Lock; 
class AddrSpace; 
class ProcMan; 

class PCB {
public: 
	PCB(Thread* input); //, int PID, Thread* parent); 
	~PCB(); 
	int getID(); 
	PCB* getParent(); 
	void setParent(PCB* parent); 
	Thread* getThread(); 
	AddrSpace* getSpace(); 
	void setSpace(AddrSpace* space); 
	bool addChild(PCB* child); 
	bool removeChild(int pid); 
	PCB* findChild(int pid); 
	int numChildren(); 
	void orphanize(); 

	Lock* lock; 
	ProcMan* children; 

private: 
	int MAX_FILES; 
	Thread* processThread; 
	int processID; 
	PCB* parent_process; 
	AddrSpace* address_space; 
};