/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/
threadnode* threadnode::head_list;
threadnode* threadnode::tail_list;
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    // Attempt to handle the situation with the blocking disk first.
if (blockingDisk != nullptr) {
    // Check if the blocking disk is ready and its queue is empty.
    if (blockingDisk->is_ready() && blockingDisk->blockQ->Isemp()) {
        // If conditions are met, retrieve the front thread and dispatch it.
        Thread *thread = blockingDisk->blockQ->popFront();
        Thread::dispatch_to(thread);
        return; // Exit the function after handling the blocking disk case.
    }
}

// If there is no blocking disk or it's not ready, check the ready queue.
if (ready_queue.head_list != nullptr) {
    // If the ready queue is not empty, retrieve and dispatch the front thread.
    Thread *thread = ready_queue.popFront();
    Thread::dispatch_to(thread);
} else {
    // If the ready queue is empty, simply return without doing anything.
    return;
}
}


void Scheduler::resume(Thread * _thread) {
    ready_queue.enqueue(_thread);
}

void Scheduler::add(Thread * _thread) {

    ready_queue.enqueue(_thread);

}

void Scheduler::terminate(Thread * _thread) {

    ready_queue.delthread(_thread);
}

void Scheduler::addition_of_Disk(BlockingDisk* bD1) {
    blockingDisk = bD1;
}

