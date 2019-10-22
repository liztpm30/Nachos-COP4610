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

void Fork (void (*func)) {
    DEBUG('a', "Fork initiated by user.\n");

    // Create pcb manager
    Procman * manager;

    // Save old registers
    currentThread->space->SaveState();
    int pid = manager->getID();

    // Create new address space & copy old to new
    AddrSpace * newSpace;
    newSpace = currentThread->space->Fork(pid);

    // Create new thread
    Thread * newThread = new Thread("Forked Thread");

    // Create pcb and associate new address
    PCB * pcb = new PCB(newThread, pid, currentThread);

    // Complete the PCB and add parent id, etc

    int pCount = machine->ReadRegister(4);

    // Copy old values into new thread
    for (int i = 0; i < numTotalRegs; i++) {
        newThread->SetUserRegister(i, currentThread->UserRegisters(i));
    }

    newThread->SetUserRegister(PCReg, pCounter);
    newThread->SetUserRegister(NextPCReg, pCounter+4);

    // Associate addressSpace to PCB
    newSpace->pcb = pcb;

    // Create process id
    printf("PID: [%d], Fork starts at address [0x%x]\n",
           currentThread->space->pcb->GetPID(), pCount);

    newThread->space = newSpace;
    
// Set new thread behavior
    newThread->Fork(func, pCount);

    // Write new process pid to r2
    machine->WriteRegister(2, pid);
}

//----------------------------------------------------------------------------
// Exec(Openfile* file)
//     Replaces the current process state with a new process executing the
//     program form a file. Returns -1 to the parent if not successful.
//     If successful, parent process is replaced with the new running
//     program from its beginning.
//---------------------------------------------------------------------------

void Exec(OpenFile* executable) {
    char path[32];
    int physAddr, pid;
    int pos = 0, cop = 0;
    ExceptionType exception;

    // Read register r4 to get executable path
    while (path[pos++] != 0) {
        while (exception != NoException) {
            exception = machine->Translate(machine->ReadRegister(4), &physAddr, 1, FALSE);
            if (exception != NoException) {
                machine->RaiseException(exception, machine->ReadRegister(4));
            }
        }
        bcopy(&machine->mainMemory[physAddr], path + cop, 1);
        cop++;
        (machine->readRegister(4))++;
    }
    path[pos] = '\0';

    DEBUG('a', "Exec[%s], initiated by user\n", path);
    printf("System Call: [%d] invoked Exec\n", currentThread->space->pcb-GetID());

    executable = fileSystem->Open(path);

    // If executable was empty
    if (executable == NULL) {
      // return -1 to register r2
        machine->WriteRegister(2, -1);
        break;
    }

    // Replace the content memory with the content of executable
    AddrSpace * execSpace = new AddrSpace(executable);

    // If new address space is empty, return -1 to register r2
    if (execSpace == NULL) {
        machine->WriteRegister(2, -1);
        break;
    }

    // Set process Id
    pid = execSpace->pcb->GetID();
    DEBUG('a', "Exec[%s], addrSpace create pid [%d]\n", path, pid);
    printf("Exeec Program: [%d] loading [%s]\n", pid, path);

    // Initialize registers
    if (pid >= 0) {
        machine->WriteRegister(2, pid);
    }
    else {
        machine->WriteRegister(2, -1);
    }

    execSpace->InitRegisters();
}
