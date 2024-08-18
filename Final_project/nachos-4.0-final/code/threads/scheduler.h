// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

enum SchedulerType {
        RR,     // Round Robin
        SJF,
        Priority
};

class Scheduler {
  public:
	Scheduler();		// Initialize list of ready threads 
	~Scheduler();				// De-allocate ready list

	void ReadyToRun(Thread* thread);	
    					// Thread can be dispatched.
	Thread* FindNextToRun();	// Dequeue first thread on the ready 
					// list, if any, and return thread.
	void Run(Thread* nextThread, bool finishing);
	    				// Cause nextThread to start running
	void CheckToBeDestroyed();	// Check if thread that had been
    					// running needs to be deleted
	void Print();			// Print contents of ready list
    
	//<REPORT>
	void insertSorted(Thread* thread);
	void UpdatePriority();
	void Aging();
	//<REPORT>
	
    // SelfTest for scheduler is implemented in class Thread
    
  private:
	SchedulerType schedulerType;
	Thread *toBeDestroyed;		// finishing thread to be destroyed
    					// by the next thread that runs
	List<Thread*> *readyListL1;  // This line declares the readyList
	List<Thread*> *readyListL2;
	List<Thread*> *readyListL3;
    //Thread *toBeDestroyed; 
	//<REPORT>

	
	//<REPORT>
};

#endif // SCHEDULER_H
