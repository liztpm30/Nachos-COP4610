// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include “synch.h”
#include “bitmap.h”

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 

    int Translate(int virtAddr);	// Converts virtAddr to physAddr
    int ReadFile(int virtAddr,		// Reads a file and loads the code
		              OpenFile* file,	// and data segments into translated
		              int size,		    // memory
		              int fileAddr);

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					                        // for now!
    unsigned int numPages;		    // Number of pages in the virtual 
					// address space
};

class MemoryManager {

public:
  MemoryManager(int numTotalPages);

  ~MemoryManager();

  /* allocates the first clear page */
  int getPage();
  
  /* Takes the index of a page and frees it.*/
  void clearPage(int pageId);

private:
  BitMap * pages;  // tracks allocation
  Lock * lock;
  
};

#endif // ADDRSPACE_H
