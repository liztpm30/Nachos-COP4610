// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "unistd.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void AdjustPC()
{
  int pc;

  pc = machine->ReadRegister(PCReg);
  machine->WriteRegister(PrevPCReg, pc);
  pc = machine->ReadRegister(NextPCReg);
  machine->WriteRegister(PCReg, pc);
  pc += 4;
  machine->WriteRegister(NextPCReg, pc);
  
}

void Exit();
void Join();

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if (which == SyscallException){
        switch (type)
        {
        case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            printf(" System Call: %d invoked Halt", getpid());
            interrupt->Halt();
            break;

        case SC_Exit:
            Exit();
            break;

        case SC_Exec:
            DEBUG('a', "Exec, initiated by user program.\n");
            printf(" System Call: %d invoked Exec", getpid());
            break;

        case SC_Join:
            Join();
            break;

        case SC_Create:
            DEBUG('a', "Create, initiated by user program.\n");
            printf(" System Call: %d invoked Create", getpid());
            break;
        
        case SC_Open:
            DEBUG('a', "Open, initiated by user program.\n");
            printf(" System Call: %d invoked Open", getpid());
            break;

        case SC_Read:
            DEBUG('a', "Read, initiated by user program.\n");
            printf(" System Call: %d invoked Read", getpid());
            break;

        case SC_Write:
            DEBUG('a', "Write, initiated by user program.\n");
            printf(" System Call: %d invoked Write", getpid());
            break;

        case SC_Close:
            DEBUG('a', "Close, initiated by user program.\n");
            printf(" System Call: %d invoked Close", getpid());
            break;

        case SC_Fork:
            DEBUG('a', "Fork, initiated by user program.\n");
            printf(" System Call: %d invoked Fork", getpid());
            break;

        case SC_Yield:
            DEBUG('a', "Yield, initiated by user program.\n");
            printf(" System Call: %d invoked Yield", getpid());
            break;
        
        default:
            printf("Unexpected system call %d\n", type);
            break;
        }
    } else {
        printf("Unexpected user mode exception %d\n", which);
        ASSERT(FALSE);
    }
    AdjustPC();
}

void Exit(){

    DEBUG('a', "Exit, initiated by user program.\n");
    printf(" System Call: %d invoked Exit", getpid());

    int status = machine->ReadRegister(4);

    printf("Process %d exits with status %d\n",getpid(),&status);

    currentThread->Finish();
}

void Join (){

    DEBUG('a', "Join, initiated by user program.\n");
    printf(" System Call: %d invoked Join", getpid());

    // Read process id from register r4
    int processId = machine->ReadRegister(4);
    int processExitId;

    // Make sure the requested process id is the child process of the current process
    /* NEED TO BE IMPLEMENT */

    //Keep on checking if the requested process is finished. if not, yield the current process
    /* NEED TO BE IMPLEMENT */

    // If the requested process finished, write the requested process exit id to register r2 to return it.

    /*save process exit ID into processExitId after PCB is implemented*/
    /* NEED TO BE IMPLEMENT */
    machine->WriteRegister(2,processExitId);

}