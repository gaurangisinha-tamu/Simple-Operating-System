/*
 File: vm_pool.C
 
 Author: Gaurangi Sinha
 Date  : March 14, 2024
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
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

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    // Assign provided values to class members
	baseAddr = _base_address;
	totalSize = _size;
	framesPool = _frame_pool;
	pagingTable = _page_table;
	pagingTable->register_pool(this);

	
	memoryRegions = (mem_region*)(baseAddr);
	allocatedRegionsCount = 1;
	memoryRegions[0].startAddr = baseAddr;
	memoryRegions[0].totalSize = Machine::PAGE_SIZE;
	for(int index = 1; index < 512; ++index) {
	    memoryRegions[index].startAddr = 0; 
	    memoryRegions[index].totalSize = 0;       
	}
    
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    Console::puts("Starting allocation...");
	if(allocatedRegionsCount > 512) {
	    Console::puts("Memory region limit reached. Cannot allocate more.");
	    assert(false); 
	}
	unsigned long requiredPages = _size/Machine::PAGE_SIZE;
	unsigned long remainingBytes = _size%Machine::PAGE_SIZE;
	if(remainingBytes > 0) requiredPages++; 
	unsigned long allocationStartAddr = memoryRegions[allocatedRegionsCount - 1].startAddr + memoryRegions[allocatedRegionsCount - 1].totalSize;
	memoryRegions[allocatedRegionsCount].startAddr = allocationStartAddr;
	memoryRegions[allocatedRegionsCount].totalSize = requiredPages * Machine::PAGE_SIZE;
	allocatedRegionsCount++; 

    Console::puts("Allocated region of memory.\n");
    return allocationStartAddr;
}

void VMPool::release(unsigned long _start_address) {
    int foundRegionIdx = -1;
for (int idx = 0; idx < 512; idx++) {
    if (memoryRegions[idx].startAddr == _start_address) {
        foundRegionIdx = idx;
        break;
    }
}

if (foundRegionIdx == -1) {
    Console::puts("Specified start address does not match any allocated regions.");
    assert(false); // Fail if the region is not found
}

unsigned long pagesToFree = memoryRegions[foundRegionIdx].totalSize / Machine::PAGE_SIZE;
unsigned long currentPageAddr = _start_address;

// Shift memory regions up to remove the freed region
for (int idx = foundRegionIdx; idx < allocatedRegionsCount - 1; idx++) {
    memoryRegions[idx] = memoryRegions[idx + 1];
}

// Clear the last allocated region
memoryRegions[allocatedRegionsCount - 1].startAddr = 0;
memoryRegions[allocatedRegionsCount - 1].totalSize = 0;

// Free each page in the region
while (pagesToFree > 0) {
    pagingTable->free_page(currentPageAddr);
    currentPageAddr += Machine::PAGE_SIZE;
    pagesToFree--;
}

// Decrement the total number of allocated regions
allocatedRegionsCount--;

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    if(_address == baseAddr) return true;
for(int i = 0; i < allocatedRegionsCount; i++) {
    if(_address >= memoryRegions[i].startAddr && _address <= (memoryRegions[i].startAddr + memoryRegions[i].totalSize)){
            return true;
        }
}

    Console::puts("Checked whether address is part of an allocated region.\n");
	return false;
}

