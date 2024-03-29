// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

Lock::Lock(char* debugName) {
    name = debugName;
    semaphore = new Semaphore("semLock", 1);
    thread = NULL;
}
Lock::~Lock() {
    delete semaphore;
}
void Lock::Acquire() {
    semaphore->P(); //waiting for the lock to become free
    thread = currentThread;
}
void Lock::Release() {
    if(isHeldByCurrentThread()){
        thread = NULL;
        semaphore->V();
    }
}

bool
Lock::isHeldByCurrentThread()
{
    return thread == currentThread;
}


//---------------------------------------------------
//    Condition::Condition
//        Initializes a condition object and takes
//        a string as the argument.
//---------------------------------------------------
 
Condition::Condition(char* debugName) {
    name = debugName;
    wQueue = new List;
}

//--------------------------------------------------
//    Condition::Condition
//        Deallocates a condition object when it
//        is no longer needed.
//--------------------------------------------------

Condition::~Condition() {
    delete wQueue;
}

//--------------------------------------------------
//    Condition::Wait
//        Waits for a condition to become free and
//        then it acquires the conditionLock for 
//        the current thread
//--------------------------------------------------

void Condition::Wait(Lock* conditionLock) {
    Semaphore *sem; 

    // Make sure there is a condition lock
    ASSERT(conditionLock->isHeldByCurrentThread());
    sem = new Semaphore("cLock", 0);    // Create sem
    wQueue->Append(sem);        // Add sem to end of list

    // Releases condition lock and adds it to current thread
    conditionLock->Release();
    sem->P();
    conditionLock->Acquire();
    delete sem;
    
}

//---------------------------------------------------
//    Condition::Signal
//        Wakes up one of the threads that is 
//        waiting on the condition
//---------------------------------------------------

void Condition::Signal(Lock* conditionLock) {
    Semaphore *sem;

    ASSERT(conditionLock->isHeldByCurrentThread());

    // Checks if waiting list is not empty and wakes 
    // up the thread
    if(!wQueue->IsEmpty()) {
        sem = (Semaphore*) wQueue->Remove();
    sem->V();
    }
}

//-----------------------------------------------------
//    Condition::Broadcast
//        Wakes up all threads that are waiting for
//        the condition
//-----------------------------------------------------

void Condition::Broadcast(Lock* conditionLock) {
    // Checks if waiting list is not empty and 
    // sends condition to signal function to wake up
    while (!wQueue->IsEmpty()) {
        Signal(conditionLock);
    }
}