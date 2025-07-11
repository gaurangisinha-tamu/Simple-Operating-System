/*
 File: ContFramePool.C
 
 Author: Gaurangi Sinha
 Date  : February 27, 2024
 
 */

/*--------------------------------------------------------------------------*/
/* 
 
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define K * 1024


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
ContFramePool* ContFramePool::start_of_frame_pool_list;
ContFramePool* ContFramePool::end_of_frame_pool_list;
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::ContFramePool(unsigned long baseFrameNumber,
                             unsigned long numberOfFrames,
                             unsigned long infoFrameNumber)
{
    // Ensure the bitmap fits in a single frame!
    assert(numberOfFrames * 4 <= FRAME_SIZE * 8);
    this->baseFrameNumber = baseFrameNumber;
    this->totalFrames = numberOfFrames;
    this->freeFrames = numberOfFrames;
    this->managementFrameNumber = infoFrameNumber;

    // Decide where to keep management info based on infoFrameNumber
    if(managementFrameNumber == 0) {
        this->bitmap = (unsigned char *) (baseFrameNumber * FRAME_SIZE);
    } else {
        this->bitmap = (unsigned char *) (infoFrameNumber * FRAME_SIZE);
    }

    // Initialize all frames to free
    for(int frameIndex = 0; frameIndex < totalFrames; frameIndex++){
        set_state(frameIndex, FrameState::Free);
    }
    // Use the first frame for management info if necessary
    if(managementFrameNumber == 0) {
        set_state(0, FrameState::HoS);
        freeFrames--;
    }
    // Link this frame pool into the list of frame pools
    if(ContFramePool::end_of_frame_pool_list == nullptr){
        ContFramePool::start_of_frame_pool_list = this;
        ContFramePool::end_of_frame_pool_list = this;
    }
    else{
        ContFramePool::end_of_frame_pool_list->nextPool = this;
        ContFramePool::end_of_frame_pool_list = this;
    }
    Console::puts("Frame Pool initialized\n");
}

void ContFramePool::print_bitmap(){
    Console::puts("Printing bitmap ===== \n");
    for(int frameIndex = 0; frameIndex < this->totalFrames; frameIndex++){
        char currentByte = bitmap[frameIndex];
        Console::puti(frameIndex);
        Console::puts(" ");
        for (int bitIndex = 7; bitIndex >= 0; bitIndex--) {
            // Display each bit in the byte
            if (currentByte & (1 << bitIndex)) {
                Console::puts("1");
            } else {
                Console::puts("0");
            }
        }
        Console::puts("\n");
        if(frameIndex == 2) break; // Example limitation for demonstration
    }
}

unsigned long ContFramePool::get_frames(unsigned int framesNeeded)
{
    assert(framesNeeded <= this->freeFrames)
    unsigned long firstFreeFrame = -1;
    for(unsigned long frameIndex = 0; frameIndex < this->totalFrames; frameIndex++){
        if(get_state(frameIndex) == FrameState::Free){
            unsigned long nextFrameIndex = frameIndex + 1;
            unsigned long contiguousFreeFrames = 1;
            while(nextFrameIndex < this->totalFrames && contiguousFreeFrames < framesNeeded){
                if(get_state(nextFrameIndex) == FrameState::Free){
                    contiguousFreeFrames++;
                    nextFrameIndex++;
                }
                else
                    break;
            }
            if(contiguousFreeFrames == framesNeeded){ // Found contiguous free frames
                firstFreeFrame = frameIndex;
                break;
            }
        }
    }
    if(firstFreeFrame != -1){
        // Allocate the contiguous frames
        set_state(firstFreeFrame, FrameState::HoS); // Mark the first frame as Head of SequenceF
        freeFrames--;
        framesNeeded--;
        unsigned long frameToAllocate = firstFreeFrame + 1;
        while(framesNeeded > 0){
            set_state(frameToAllocate, FrameState::Used);
            frameToAllocate++;
            framesNeeded--;
            freeFrames--;
        }

        return (firstFreeFrame + baseFrameNumber);
    }
    else
        return 0; // Allocation failed
}

void ContFramePool::mark_frames_inaccessible(unsigned long baseFrameNumber,
                                             unsigned long framesToMark)
{
    for (int frameIndex = baseFrameNumber; frameIndex < baseFrameNumber + framesToMark; frameIndex++){
        set_state(frameIndex - this->baseFrameNumber, FrameState::Inaccessible);
    }
}

void ContFramePool::release_frames(unsigned long firstFrameNumber)
{
    ContFramePool* poolIterator = ContFramePool::start_of_frame_pool_list;
    bool poolFound = false;

    // Find the pool that contains the firstFrameNumber
    while(poolIterator != nullptr){
        if(poolIterator->baseFrameNumber <= firstFrameNumber && firstFrameNumber < poolIterator->baseFrameNumber + poolIterator->totalFrames){
            poolFound = true;
            break;
        }
        poolIterator = poolIterator->nextPool;
    }

    if(poolFound){
        unsigned long frameIndex = firstFrameNumber - poolIterator->baseFrameNumber;
        if(poolIterator->get_state(frameIndex) != FrameState::HoS){
            Console::puts("Error: First frame provided is not marked as Head of Sequence");
        }
        else{
            // Release the sequence of allocated frames starting with the HoS frame
            poolIterator->set_state(frameIndex, FrameState::Free);
            poolIterator->freeFrames++;
            int nextFrameIndex = frameIndex + 1;
            while(poolIterator->get_state(nextFrameIndex) == FrameState::Used){
                poolIterator->set_state(nextFrameIndex, FrameState::Free);
                nextFrameIndex++;
                poolIterator->freeFrames++;
            }
        }
    }
    else{
        Console::puts("Error: Frame number not found in any frame pool");
    }
}

unsigned long ContFramePool::needed_info_frames(unsigned long frames)
{
    // Calculate the number of frames needed to store info based on a size of 16K per frame
    return frames / (16 * 1024) + (frames % (16 * 1024) > 0 ? 1 : 0);
}

ContFramePool::FrameState ContFramePool::get_state(unsigned long frameNumber){
    unsigned int index = frameNumber / 4;
    unsigned int position = (frameNumber % 4) * 2;
    unsigned int mask = 0x1 << position;
    if((bitmap[index] & mask) == 0){
        mask <<= 1;
        if((bitmap[index] & mask) == 0)
            return FrameState::Inaccessible;
        return FrameState::Used;
    } else {
        mask <<= 1;
        if((bitmap[index] & mask) == 0)
            return FrameState::HoS;
        return FrameState::Free;
    }
}

void ContFramePool::set_state(unsigned long frameNumber, FrameState state) {
    unsigned int index = frameNumber / 4;
    unsigned int position = (frameNumber % 4) * 2;
    unsigned int mask = 0x3 << position; // Prepare a mask for clearing or setting bits
    switch(state){
        case FrameState::Free:
            bitmap[index] |= mask; // Set bits to indicate 'Free'
            break;
        case FrameState::Used:
            bitmap[index] &= ~mask; // Clear bits to indicate 'Used'
            bitmap[index] |= (0x2 << position); // Ensure the second bit is set
            break;
        case FrameState::HoS:
            bitmap[index] &= ~mask; // Clear bits first
            bitmap[index] |= (0x1 << position); // Set the first bit
            break;
        case FrameState::Inaccessible:
            bitmap[index] &= ~mask; // Clear both bits to indicate 'Inaccessible'
            break;
    }
}




