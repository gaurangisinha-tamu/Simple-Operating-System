#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   page_directory = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);
    auto* page_table = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);

        // Initialize page table with default attributes
        unsigned long base_addr = 0;
        for (int idx = 0; idx < 1024; ++idx) {
            page_table[idx] = base_addr | 3; // Setting up: read/write, present
            base_addr += PAGE_SIZE;
        }

        // Setting the first entry of the directory to point to the table
        page_directory[0] = (unsigned long) page_table | 3;
        // Mark remaining directory entries as not present
        for (int entry = 1; entry < 1024; ++entry) {
            page_directory[entry] = 0x2; // Attributes: supervisor level, read/write, not present
        }
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long) page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   paging_enabled = true;
   write_cr0(read_cr0() | 0x80000000);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  unsigned int faultCode = _r->err_code;
    // Check if the error code indicates a protection fault
    if (faultCode & 1) {
        Console::puts("Detected Protection Fault, NOT a page fault.");
        return;
    }

    // Retrieve the faulting address and the current page directory
    unsigned long faultAddr = read_cr2();
    unsigned long* dirAddr = reinterpret_cast<unsigned long*>(read_cr3());

    // Define masks for isolating page table and directory indices
    const unsigned long tblIdxMask = 0x003ff000;
    const unsigned long dirIdxMask = 0xffc00000;

    // Calculate indices in the page directory and table
    unsigned long tblIdx = (faultAddr & tblIdxMask) >> 12;
    unsigned long dirIdx = (faultAddr & dirIdxMask) >> 22;

    unsigned long* page_table;
    bool isNewTbl = false;

    // Check if the page table needs to be allocated
    if (!(dirAddr[dirIdx] & 1)) {
        dirAddr[dirIdx] = (unsigned long)(kernel_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
        isNewTbl = true;
    }
    page_table = reinterpret_cast<unsigned long*>(dirAddr[dirIdx] & ~0xFFF); // Mask to get the table address

    // Initialize a newly allocated page table
    if (isNewTbl) {
        for (int i = 0; i < 1024; ++i) {
            page_table[i] = 4; // Mark as not present but writable
        }
    }

    // Allocate a frame for the faulting address and update the table
    page_table[tblIdx] = (unsigned long)(process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;

  Console::puts("handled page fault\n");
}

