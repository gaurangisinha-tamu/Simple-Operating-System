/*
 File: scheduler.C
 
 Author: Gaurangi Sinha
 Date  : 28th March, 2024
 
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

/* -- (none) -- */

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
  bool interrupts_were_enabled = Machine::interrupts_enabled();
  if(interrupts_were_enabled) Machine::disable_interrupts();
  if(ready_queue.head_list == nullptr) {
    if(interrupts_were_enabled) Machine::enable_interrupts();
    return;
  }
  Thread *thread = ready_queue.popFrontThread();
  Thread::dispatch_to(thread);
  if(interrupts_were_enabled) Machine::enable_interrupts();
}

void Scheduler::resume(Thread * _thread) {
  if(Machine::interrupts_enabled()) Machine::disable_interrupts();
    ready_queue.enqueue(_thread);
    Machine::enable_interrupts();
}

void Scheduler::add(Thread * _thread) {
  bool interrupts_were_enabled = Machine::interrupts_enabled();
  if(interrupts_were_enabled) Machine::disable_interrupts();
  ready_queue.enqueue(_thread);
  if(interrupts_were_enabled) Machine::enable_interrupts();
}

void Scheduler::terminate(Thread * _thread) {
  bool interrupts_were_enabled = Machine::interrupts_enabled();
  if(interrupts_were_enabled) Machine::disable_interrupts();

  ready_queue.removeThread(_thread);

  if(interrupts_were_enabled) Machine::enable_interrupts();
}
