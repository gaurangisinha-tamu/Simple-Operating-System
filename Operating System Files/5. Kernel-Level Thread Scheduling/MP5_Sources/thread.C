/* 
    File: thread.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 11/10/25


    This code does the low-level management of kernel-level threads.
    It supports creation of threads and low-level dispatching.

    It has been developed after study of David H. Hovemeyer's "kthread"
    code <daveho@cs.umd.edu> and uses his approach for setting up the 
    stack of the thread upon start-up. 

    Some portions of his code have been derived from David Hovemeyer's 
    code.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"

#include "frame_pool.H"

#include "thread.H"

#include "threads_low.H"
#include "scheduler.H"
/*--------------------------------------------------------------------------*/
/* EXTERNS */
/*--------------------------------------------------------------------------*/
extern Scheduler * SYSTEM_SCHEDULER;

Thread * current_thread = 0;
/* Pointer to the currently running thread. This is used by the scheduler,
   for example. */

/* -------------------------------------------------------------------------*/
/* LOCAL DATA PRIVATE TO THREAD AND DISPATCHER CODE */
/* -------------------------------------------------------------------------*/

int Thread::nextFreePid;

/* -------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS */
/* -------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------*/
/* EXPLICIT STACK OPERATIONS */

inline void Thread::push(unsigned long _val) {
    /* This function is originally borrowed from David H. Hovemeyer <daveho@cs.umd.edu> */
    esp -= 4;
    *((unsigned long *) esp) = _val;
}

/* -------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS TO START/SHUTDOWN THREADS. */

static void thread_shutdown() {
    /* This function should be called when the thread returns from the thread function.
       It terminates the thread by releasing memory and any other resources held by the thread. 
       This is a bit complicated because the thread termination interacts with the scheduler.
     */
	SYSTEM_SCHEDULER->terminate(current_thread);
    delete current_thread;
    SYSTEM_SCHEDULER->yield();

    
    /* Let's not worry about it for now. 
       This means that we should have non-terminating thread functions. 
    */
}

static void thread_start() {
     /* This function is used to release the thread for execution in the ready queue. */
    Machine::enable_interrupts();
     /* We need to add code, but it is probably nothing more than enabling interrupts. */
}

void Thread::setup_context(Thread_Function _tfunction){
    /* Sets up the initial context for the given kernel-only thread. 
       The thread is supposed the call the function _tfunction upon start.
    */
  
    /* The approach and most of the code in this function are borrowed from 
       David H. Hovemeyer <daveho@cs.umd.edu> */
 
    /* -- HERE WE PUSH THE ITEMS ON THE STACK THAT ARE NEEDED FOR THE
          THREAD TO START EXECUTION AND FOR IT TO TERMINATE CORRECTLY
          WHEN THE THREAD FUNCTION RETURNS. */

    /* ---- ARGUMENT TO THREAD FUNCTION */
    push(0); /* At this point we don't have arguments. */

    /* ---- ADDRESS OF SHUTDOWN FUNCTION */
    push((unsigned long) &thread_shutdown);
    /* The thread_shutdown function should be called when the thread function 
       returns. */

    /* Push the address of the thread function. */
    push((unsigned long) _tfunction);

    /* -- NOW WE NEED TO MAKE THE REST OF THE STACK LOOK LIKE AFTER AN EXCEPTION. */
    /*
     * The EFLAGS register will have all bits clear.
     * The important constraint is that we want to have the IF
     * bit clear, so that interrupts are disabled when the
     * thread starts.
     */
    /* ---- EFLAGS */
    push(0);
    /* Clear the IF bit to disable interrupts when thread starts. */

    /* ---- CS and EIP REGISTERS */
    push(Machine::KERNEL_CS);
    push((unsigned long) &thread_start);
    /* In the instruction pointer (EIP) we store address of the 
       function that will kick-start the thread. */

    /* Push fake error code and interrupt number. */
    push(0);
    push(0);

    /* Push initial values for general-purpose registers. */
    push(0);  /* eax */
    push(0);  /* ecx */
    push(0);  /* edx */
    push(0);  /* ebx */
    push(0);  /* esp */
    push(0);  /* ebp */
    push(0);  /* esi */
    push(0);  /* edi */

    /*
     * Push values for saved segment registers.
     * Only the ds and es registers will contain valid selectors.
     * The fs and gs registers are not used by any instruction
     * generated by gcc.
     */
    push(Machine::KERNEL_DS);  /* ds */
    push(Machine::KERNEL_DS);  /* es */
    push(0);  /* fs */
    push(0);  /* gs */

    Console::puts("esp = "); Console::putui((unsigned int)esp); Console::puts("\n");

    Console::puts("done\n");
}

/*--------------------------------------------------------------------------*/
/* -- Thread CONSTRUCTOR -- */
/*--------------------------------------------------------------------------*/

Thread::Thread(Thread_Function _tf, char * _stack, unsigned int _stack_size) {
/* Construct a new thread and initialize its stack. The thread is then ready to run.
   (The dispatcher is implemented in file "thread_scheduler".) 
*/

    /* -- INITIALIZE THREAD */

    /* ---- THREAD ID */
   
    thread_id = nextFreePid++;

    /* ---- STACK POINTER */

    esp = (char*)((unsigned int)_stack + _stack_size);
    /* RECALL: The stack starts at the end of the reserved stack memory area. */

    stack = _stack;
    stack_size = _stack_size;
    
    /* -- INITIALIZE THE STACK OF THE THREAD */

    setup_context(_tf);

}

int Thread::ThreadId() {
    return thread_id;
}

void Thread::dispatch_to(Thread * _thread) {
/* Context-switch to the given thread. Calls the low-level context switch code 
   in thread_low.asm.
   NOTE: This call does not return until after the current thread is switched back in.
   NOTE: We don't consider the system start thread as an actual thread. Therefore, we will
         not return from this function ever when the system start code (in kernel.C) starts up 
         the first thread.
*/

    /* The value of 'current_thread' is modified inside 'threads_low_switch_to()'. */

    threads_low_switch_to(_thread);

    /* The call does not return until after the thread is context-switched back in. */
}
       

Thread * Thread::CurrentThread() {
/* Return the currently running thread. */
    return current_thread;
}
