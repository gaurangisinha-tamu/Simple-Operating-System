/*
 File: cont_frame_pool.H

 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 17/02/04

 Description: Management of the CONTIGUOUS Free-Frame Pool.

 As opposed to a non-contiguous free-frame pool, here we can allocate
 a sequence of CONTIGUOUS frames.

 */

#ifndef _CONT_FRAME_POOL_H_                   // include file only once
#define _CONT_FRAME_POOL_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* C o n t F r a m e   P o o l  */
/*--------------------------------------------------------------------------*/

class ContFramePool {
    
private:
    /* -- DEFINE YOUR CONT FRAME POOL DATA STRUCTURE(s) HERE. */
    unsigned char* bitmap;             // Bitmap for frame management
    unsigned int freeFrames;            // Number of free frames in the pool
    unsigned long baseFrameNumber;          // Starting frame number in physical memory
    unsigned long totalFrames;              // Total number of frames in the pool
    unsigned long managementFrameNumber;    // Frame number for management info

    /* ---- STATE MANAGEMENT */

    enum class FrameState { Free, Used, HoS, Inaccessible };

    // Static pointers to manage list of frame pools
    static ContFramePool* start_of_frame_pool_list,*end_of_frame_pool_list;
    ContFramePool* nextPool;                // Next frame pool in the list
    FrameState get_state(unsigned long frameNumber);
    void set_state(unsigned long frameNumber, FrameState state);

public:

    // Frame size matches page size
    //FrameState get_state(unsigned long frameNumber);
    static const unsigned int FRAME_SIZE = Machine::PAGE_SIZE;

    ContFramePool(unsigned long baseFrameNumber,
                  unsigned long numberOfFrames,
                  unsigned long infoFrameNumber);
    /*
     Initializes the frame pool.
     baseFrameNumber: Start frame number managed by this pool.
     numberOfFrames: Size of this frame pool, in frames.
     infoFrameNumber: Frame number for storing management information.
     Note: If infoFrameNumber is 0, any frame in the pool can be used for management.
     */

    unsigned long allocate_frames(unsigned int framesNeeded);
    /*
     Allocates contiguous frames from the pool.
     framesNeeded: Number of contiguous frames required.
     Returns the first frame number if successful, or 0 if it fails.
     */
    
    void mark_frames_inaccessible(unsigned long baseFrameNumber,
                                  unsigned long framesToMark);
    /*
     Marks a contiguous sequence of frames as inaccessible.
     baseFrameNumber: First frame to mark.
     framesToMark: Number of contiguous frames to mark.
     */

    void print_bitmap();
    /*
     * Utility to print the frame allocation bitmap.
     */

    static void release_allocated_frames(unsigned long firstFrameNumber);
    /*
     Releases a sequence of allocated frames back to the pool.
     Identified by the number of the first frame in the sequence.
     */

    static unsigned long calculate_needed_info_frames(unsigned long frames);
    /*
     Calculates frames needed for pool management information.
     Depends on pool implementation and frame size.
     */

};
#endif

