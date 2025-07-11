/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"

extern Scheduler *SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    this->blockQ = new threadnode();
}

void BlockingDisk::wait_until_ready() {
    // Check the readiness of the Blocking Disk before proceeding.
if (!BlockingDisk::is_ready()) {
    // Output a message indicating that the disk is not yet ready.
    Console::puts("The disk is currently unready; the IO thread is now yielding execution.\n");

    // Enqueue the current thread into the block queue for later processing.
    blockQ->enqueue(Thread::CurrentThread());

    // Yield the current thread to allow other processes to run.
    SYSTEM_SCHEDULER->yield();
}

}

bool BlockingDisk::is_ready() {
    // Read disk status from hardware port and check ready bit
    int status = Machine::inportb(0x1F7);
    bool diskReady = (status & 0x08) != 0;

    return diskReady;
}


void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
    wait_until_ready();
    SimpleDisk::issue_operation(DISK_OPERATION::READ, _block_no);
    wait_until_ready();

    /* read data from port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i*2]   = (unsigned char)tmpw;
        _buf[i*2+1] = (unsigned char)(tmpw >> 8);
    }
    Console::puts("Reading data is Complete\n");

}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
    wait_until_ready();

    SimpleDisk::issue_operation(DISK_OPERATION::WRITE, _block_no);

    wait_until_ready();

    /* write data to port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
        Machine::outportw(0x1F0, tmpw);
    }
    Console::puts("The writing of data is complete\n");

}
