// threadtest.cc 
//      Simple test case for the threads assignment.
//
//      Create two threads, and have them context switch
//      back and forth between themselves by calling Thread::Yield, 
//      to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
//      Loop 5 times, yielding the CPU to another ready thread 
//      each iteration.
//
//      "which" is simply a number identifying the thread, for debugging
//      purposes.
//----------------------------------------------------------------------

int SharedVariable;
Semaphore * sem = new Semaphore("mySem", 1);


void
SimpleThread(int which) {

int i, num, val;
for(num = 0; num < 5; num++) {
sem ->P();
val = SharedVariable;
printf("*** thread %d sees value %d\n", which, val);
currentThread->Yield();
SharedVariable = val+1;
sem->V();
currentThread->Yield();
}

sem->P();
sem->V();
sem->P();
sem->V();
sem->P();
sem->V();
sem->P();
sem->V();

val = SharedVariable;

printf("Thread %d sees final value %d\n", which, val);
}

// ThreadTest1
//      Set up a ping-pong between two threads, by forking a thread 
//      to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1(int n)
{

    int num;
    DEBUG('t', "Entering ThreadTest1");

    for(num = 1; num <= n; num++) {

        Thread *t = new Thread("forked thread");

        t->Fork(SimpleThread, num);
    }

    SimpleThread(0);

}

//----------------------------------------------------------------------
// ThreadTest
//      Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest(int n)
{
    switch (testnum) {
    case 1:
        ThreadTest1(n);
        break;
    default:
        printf("No test specified.\n");
        break;
    }
}
/*******************
* ELEVATOR SECTION *
*******************/ 
#ifdef HW1_ELEVATOR 

#ifndef HW1_SEMAPHORES
Semaphore *s = new Semaphore("beep boop", 1);
#endif 

/* CONFIGURATION VARIABLES */
int personNumber = 0; 
const int numFloors = 5; 
const int elevatorSize = 5; 
Condition* ellyCond; 
Lock* ellyLock; 
int currentFloor, direction, people; 

struct PersonThread {
	int id; 
	int atFloor; 
	int toFloor; 
};

struct ElevatorThread {
	int numFloors; 
	int currentFloor; 
	int numPeopleIn; 
	int direction = 1; //1 = up, 0 = down 
};

struct FloorData {
	int onReqs; 
	int offReqs; 
} *building;

void startup(int numFloors) {
	int currentFloor = 0, direction = 1, people = 0; 
	building = new struct FloorData[numFloors]; 
	for(int i = 0; i < numFloors; i++) {
		building[i].onReqs = 0; 
		building[i].offReqs = 0; 
	}
	ellyLock = new Lock("Elevator Lock"); 
	ellyCond = new Condition("Elevator Condition"); 
}

void runElly(int numFloors) {
	while(true) {
		ellyLock->Acquire(); 
		for(int i = 0; i < 50; i++); //elevator music
		if(direction) currentFloor++; 
		else currentFloor--; 
		printf("Elevator arrives at floor %d.\n", currentFloor+1);
		//have to offset currentFloor by 1 (0 is the first floor, etc.) 
		ellyLock->Release(); 
		currentThread->Yield(); 
		if(currentFloor + 1 == numFloors) direction = 0; 
		else if(currentFloor == 0) direction = 1; 
		ellyLock->Acquire(); 
		ellyCond->Broadcast(ellyLock); //see if anybody wants off
		ellyLock->Release(); 
		ellyLock->Acquire(); //start letting them off
		while(building[currentFloor].offReqs) ellyCond->Wait(ellyLock); 
		ellyLock->Release(); 
		ellyLock->Acquire(); 
		ellyCond->Broadcast(ellyLock); //see if anybody wants on
		ellyLock->Release(); 
		ellyLock->Acquire(); //start letting them on
		while(building[currentFloor].onReqs && people < elevatorSize) 
			ellyCond->Wait(ellyLock); 
		ellyLock->Release(); 
	}
}

void runPerson(int p) {
	PersonThread* P = (PersonThread*) p; 
	int at = P->atFloor, to = P->toFloor, id = P->id; 
	//make request
	ellyLock->Acquire(); 
	building[at].onReqs++; 
	ellyLock->Release(); 
	//wait patiently? 
	ellyLock->Acquire(); 
	while(at-1!=currentFloor || people == elevatorSize)
		ellyCond->Wait(ellyLock);
	ellyLock->Release(); 
	ellyLock->Acquire(); 
	building[to-1].offReqs++; 
	building[at-1].onReqs--; 
	people++; 
	printf("Person %d got into the elevator at floor %d.\n", id, currentFloor+1);
	ellyCond->Broadcast(ellyLock); 
	ellyLock->Release(); 
	ellyLock->Acquire(); 
	while(currentFloor!=to-1) ellyCond->Wait(ellyLock); 
	people--; 
	building[to-1].offReqs--; 
	printf("Person %d got out of the elevator at floor %d.\n", id, currentFloor+1);
	ellyCond->Broadcast(ellyLock); 
	ellyLock->Release(); 
}

void Elevator(int numFloors) {
	Thread* ellyThread = new Thread("The Elevator"); 
	startup(numFloors); 
	ellyThread->Fork(runElly, numFloors); 
}

PersonThread* ArrivingGoingFromTo(int atFloor, int toFloor) {
	Thread* pThread = new Thread("Personal Thread");
	PersonThread* p = new PersonThread; 
	p->id = personNumber++; 
	p->atFloor = atFloor; 
	p->toFloor = toFloor; 
	printf("Person %d wants to go to floor %d from floor %d.\n", 
		p->id, p->atFloor, p->toFloor);
	pThread->Fork(runPerson, (int) p); 
	return p; 
}
#endif 











