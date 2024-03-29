// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = i;
	pageTable[i].valid = TRUE;
	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

//----------------------------------------------------------------------
// AddrSpace::Translate
//      Converts a virtual address to a physical address. It uses
//      a page table. First, it checks for alignment errors and then
//      if everything is ok, it sets the dirty bits in the table
//      entry and stores the translated physical address in 'physAddr'.
//      Returns exception if there's an error.
//
//      "virtAddr" - the virtual address to translate
//      "physAddr" - the place to store teh physical address
//      "size" - the amount of memory being read or written
//----------------------------------------------------------------------

int AddrSpace::Translate(int virtAddr)
{
    // Create a physical address
    int size;
    int physAddr;
    bool writing;
    int vpn = (unsigned) virtAddr / PageSize;
    int offset = (unsigned) virtAddr % PageSize;
    TranslationEntry *entry;
    unsigned int pageFrame;

    DEBUG('a', "\tTranslate 0x%x, %s: ", virtAddr, writing ?
                "write" : "read");

    // Calculate the page number and offset within the page from
    // the virtual address
    if (vpn >= numPages) {
        DEBUG('a', "Virtual page # %d too large for numPages %d!\n",
            virtAddr, numPages);
        return -1;
    }
    if (!pageTable[vpn].valid) {
        DEBUG('a', "Virtual page # %d is not valid!\n",
            virtAddr);
        return -1;
    }
    entry = &pageTable[vpn];

    pageFrame = entry->physicalPage;

    // set the use, dirty bits
    entry->use = TRUE;
    if (writing)
        entry->dirty = TRUE;
    physAddr = pageFrame * PageSize + offset;
    DEBUG('a', "translate vpn %d to pfn %d\n", virtAddr, physAddr);

    return physAddr;
}

//----------------------------------------------------------------------
// AddrSpace::ReadFile
//      Loads the code from a file and data segments into the translated
//      memory, instead of at position 0.
//
//      "virtAddr" - the virtual address to translate
//      "physAddr" - the place to store teh physical address
//      "size" - the amount of memory being read or written
//      "file" - file that holds the code
//----------------------------------------------------------------------

int
AddrSpace::ReadFile(int virtAddr, OpenFile* file, int size, int fileAddr)
{
    char buff[size];
    int currSize = file->ReadAt(buff, size, fileAddr);
    int currSizeCopy = currSize;
    int copied = 0, newSize = 0, phyAddr;

    while (currSizeCopy > 0) {
        // convert the virtAddr given into phyAddr
        phyAddr = Translate(virtAddr);

        newSize = min(PageSize, currSizeCopy);
        bcopy(buff+copied, &machine->mainMemory[phyAddr], newSize);

        virtAddr = virtAddr + newSize;
        currSizeCopy = currSizeCopy - newSize;
        copied = copied + newSize;
    }
    return currSize;
}

//----------------------------------------------------------------------
// Memory Manager Implementation
//----------------------------------------------------------------------

/* Constructor */

MemoryManager::MemoryManager(int numTotalPages) {  
  
  pages = new BitMap(numpages);
  lock = new Lock("Memory Manager");
}

/* Destructor */

MemoryManager::~MemoryManager() {
  delete pages;
  pages = 0;
  delete lock;  
}

/* Page allocation */

int MemoryManager::getPage() {
  lock->Acquire();
  int page = pages->Find();  
  lock->Release();

  return page;
}

/* Clear page */

void MemoryManager::clearPage(int pageId) {
  lock->Acquire();
  pages->Clear(physPageNum);
  lock->Release();
}
