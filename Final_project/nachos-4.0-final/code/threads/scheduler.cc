// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"
#include "list.h"
//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------



//<TODO>
// Declare sorting rule of SortedList for L1 & L2 ReadyQueue
// Hint: Funtion Type should be "static int"
/*
static int CompareByRemainingTime(const Thread* a, const Thread* b) {
    // Example comparison logic:
    return a->getRemainingBurstTime() - b->getRemainingBurstTime();
}

static int CompareByArrival(const Thread* a, const Thread* b) {
    // Example comparison logic:
    // Assume 'arrivalTime' is a member that stores the thread's arrival time
    return a->arrivalTime - b->arrivalTime;
}
*/

int Thread::getQueueLevel() const {
    if (Priority < 50) {
        return 3; // L3 queue
    } else if (Priority < 100) {
        return 2; // L2 queue
    } else {
        return 1; // L1 queue
    }
}

// Define this in an appropriate location that's accessible when you instantiate the SortedList
int CompareThreadsByRemainingTime(Thread* a, Thread* b) {
    // Ensure that you safely handle possible null pointers if necessary
    if (!a || !b) return 0; // or handle differently based on your logic

    return a->getRemainingBurstTime() - b->getRemainingBurstTime();
}


//<TODO>

Scheduler::Scheduler()
{
//	schedulerType = type;
    //readyList = new List<Thread *>; 
    //<TODO>
    // Initialize L1, L2, L3 ReadyQueue
    //<TODO>
	readyListL1 = new SortedList<Thread*>(CompareThreadsByRemainingTime); // SRTN
    readyListL2 = new List<Thread *>(); // FCFS
    readyListL3 = new List<Thread *>(); // Round Robin
} 


//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    //<TODO>
    // Remove L1, L2, L3 ReadyQueue
    delete readyListL1;
    delete readyListL2;
    delete readyListL3;
    //<TODO>
    //delete readyList; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    // DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());

    Statistics* stats = kernel->stats;
    //<TODO>
    // According to priority of Thread, put them into corresponding ReadyQueue.
    // After inserting Thread into ReadyQueue, don't forget to reset some values.
    // Hint: L1 ReadyQueue is preemptive SRTN(Shortest Remaining Time Next).
    // When putting a new thread into L1 ReadyQueue, you need to check whether preemption or not.

    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    //DEBUG('z', "Tick [%d]: Thread [%d] is inserted into queue L[%d].\n", stats->totalTicks, thread->getThreadID(), thread->getQueueLevel());
    char buffer[256];
    sprintf(buffer, "[InsertToQueue] Tick [%d]: Thread [%d] is inserted into queue L[%d]",
            stats->totalTicks, thread->getID(), thread->getQueueLevel());
    DEBUG('z', buffer);


    switch (thread->getQueueLevel()) {
        case 1:
            static_cast<SortedList<Thread*>*>(readyListL1)->Insert(thread); // Cast to SortedList and use Insert
            break;
        case 2:
            readyListL2->Append(thread);
            break;
        case 3:
            readyListL3->Append(thread);
            break;
    }

    thread->setStatus(READY);
    (void) kernel->interrupt->SetLevel(oldLevel);
    //<TODO>
    // readyList->Append(thread);
}



//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    /*if (readyList->IsEmpty()) {
    return NULL;
    } else {
        return readyList->RemoveFront();
    }*/

    //<TODO>
    // a.k.a. Find Next (Thread in ReadyQueue) to Run
    Thread* nextThread = NULL;

    if (!readyListL1->IsEmpty()) {
        nextThread = readyListL1->RemoveFront();
    } else if (!readyListL2->IsEmpty()) {
        nextThread = readyListL2->RemoveFront();
    } else if (!readyListL3->IsEmpty()) {
        nextThread =  readyListL3->RemoveFront();
    } 
    char buffer[256];
    sprintf(buffer, "[RemoveFromQueue] Tick [%d]: Thread [%d] is removed from queue L[%d]",
            kernel->stats->totalTicks, nextThread->getID(), nextThread->getQueueLevel());
    DEBUG('z', buffer);

    return nextThread; 
    //<TODO>
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
 
//	cout << "Current Thread" <<oldThread->getName() << "    Next Thread"<<nextThread->getName()<<endl;
   
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	     toBeDestroyed = oldThread;
    }
   
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (oldThread->space != NULL) {	// if this thread is a user program,

        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
#endif
    //Thread *oldThread = kernel->currentThread;
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    if (oldThread->getStatus() == RUNNING) {
        oldThread->setStatus(READY);
    }

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    kernel->currentThread = nextThread;
    // DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    //DEBUG('z', "Tick [%d]: Thread [%d] is now selected for execution, thread [%d] is replaced, and it has executed [%d] ticks.\n", 
        //stats->totalTicks, nextThread->getThreadID(), oldThread->getThreadID(), oldThread->getTotalTicks());
    char buffer[256];
    sprintf(buffer, "[ContextSwitch] Tick [%d]: Thread [%d] is now selected for execution, thread [%d] is replaced, and it has executed [%d] ticks",
            kernel->stats->totalTicks, nextThread->getID(), oldThread->getID(), oldThread->getRunTime());
    DEBUG('z', buffer);

    cout << "Switching from: " << oldThread->getID() << " to: " << nextThread->getID() << endl;
    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << kernel->currentThread->getID());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
#ifdef USER_PROGRAM
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	    oldThread->space->RestoreState();
    }
#endif
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        DEBUG(dbgThread, "toBeDestroyed->getID(): " << toBeDestroyed->getID());
        delete toBeDestroyed;
	    toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    // readyList->Apply(ThreadPrint);
    readyListL1->Apply(ThreadPrint);
    readyListL2->Apply(ThreadPrint);
    readyListL3->Apply(ThreadPrint);
}

// <TODO>

// Function 1. Function definition of sorting rule of L1 ReadyQueue

// Function 2. Function definition of sorting rule of L2 ReadyQueue

// Function 3. Scheduler::UpdatePriority()
// Hint:
// 1. ListIterator can help.
// 2. Update WaitTime and priority in Aging situations
// 3. After aging, Thread may insert to different ReadyQueue

void 
Scheduler::UpdatePriority()
{

}

// <TODO>

// Implement the aging mechanism
void Scheduler::Aging() {
    ListIterator<Thread *> *it1 = new ListIterator<Thread *>(readyListL1);
    ListIterator<Thread *> *it2 = new ListIterator<Thread *>(readyListL2);
    ListIterator<Thread *> *it3 = new ListIterator<Thread *>(readyListL3);

    while (!it1->IsDone()) {
        Thread *thread = it1->Item();
        if (kernel->stats->totalTicks - thread->getRRTime()  > 400) {
            int oldPriority = thread->getPriority();
            thread->setPriority(oldPriority + 10);
            //DEBUG('z', "Tick [%d]: Thread [%d] changes its priority from [%d] to [%d].\n", 
                //stats->totalTicks, thread->getThreadID(), oldPriority, thread->getPriority());
            char buffer[256];
            sprintf(buffer, "[UpdatePriority] Tick [%d]: Thread [%d] changes its priority from [%d] to [%d]",
                    kernel->stats->totalTicks, thread->getID(), oldPriority, thread->getPriority());
            DEBUG('z', buffer);
        }
        it1->Next();
    }

    while (!it2->IsDone()) {
        Thread *thread = it2->Item();
        //rest of code
        if (kernel->stats->totalTicks - thread->getRRTime() > 400) {
            int oldPriority = thread->getPriority();
            thread->setPriority(oldPriority + 10);
            //DEBUG('z', "Tick [%d]: Thread [%d] changes its priority from [%d] to [%d].\n", 
            //stats->totalTicks, thread->getThreadID(), oldPriority, thread->getPriority());
            char buffer[256];
            sprintf(buffer, "[UpdatePriority] Tick [%d]: Thread [%d] changes its priority from [%d] to [%d]",
                    kernel->stats->totalTicks, thread->getID(), oldPriority, thread->getPriority());
        }
        it2->Next();
    }

    while (!it3->IsDone()) {
        Thread *thread = it3->Item();
        if (kernel->stats->totalTicks - thread->getRRTime() > 400) {
            int oldPriority = thread->getPriority();
            thread->setPriority(oldPriority + 10);
            //DEBUG('z', "Tick [%d]: Thread [%d] changes its priority from [%d] to [%d].\n", 
            //stats->totalTicks, thread->getThreadID(), oldPriority, thread->getPriority());
            char buffer[256];
            sprintf(buffer, "[UpdatePriority] Tick [%d]: Thread [%d] changes its priority from [%d] to [%d]",
                    kernel->stats->totalTicks, thread->getID(), oldPriority, thread->getPriority());
        }
        it3->Next();
    }
}