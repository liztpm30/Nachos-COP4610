#include "pcb.h"

PCB::PCB(Thread* input) {
	processThread = input; 
	processID = pid_manager->getPid(); 
	parent_process = NULL; 
	MAX_FILES = 16; //arbitrary? 
	children = new ProcMan(); 
}

PCB::~PCB() {
	delete lock; 
	delete children; 
}

int PCB::getID() {
	return processID; 
}

PCB* PCB::getParent() {
	return parent_process; 
}

void PCB::setParent(PCB* parent) {
	parent_process = parent; 
}

Thread* PCB::getThread() {
	return processThread; 
}

AddrSpace* PCB::getSpace() {
	return address_space; 
}

void PCB::setSpace(AddrSpace* space) {
	address_space = space; 
}

bool PCB::addChild(PCB* child) {
	return children->addPCB(child); 
}

bool PCB::removeChild(int pid) {
	return children->removePCB(pid); 
}

PCB* PCB::findChild(int pid) {
	return children->getPCB(pid); 
}

int PCB::numChildren() {
	return children->getProcCount(); 
}

void PCB::orphanize() {
	return children->orphanize(); 
}