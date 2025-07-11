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
VMPool *PageTable::vm[];


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
    // Initialize a kernel memory pool for managing page tables and directories
page_directory = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);
    auto* page_table = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);

// Define the initial memory address
unsigned long baseAddr = 0;

// Setup page table entries
for(int idx = 0; idx < 1024; ++idx) {
    // Mark each page as present and read/write
    page_table[idx] = baseAddr | 0x03; // OR operation with 3 sets lowest two bits (present and read/write)
    baseAddr += PAGE_SIZE;
}

// Link page table to the first entry of the page directory
page_directory[0] = (unsigned long) page_table | 0x03;

// Configure the remaining entries of the page directory
for(int dir_idx = 1; dir_idx < 1024; ++dir_idx) {
    // Last entry points to the directory itself, to form a recursive mapping
    if(dir_idx == 1023) {
        page_directory[dir_idx] = (unsigned long) page_directory | 0x03;
    } else {
        // Set pages to supervisor level, read/write, not present
        page_directory[dir_idx] = 0x02; // Only the write bit is set
    }
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

unsigned long * PageTable::PDE_address(unsigned long addr) {
    unsigned long pde_index = addr >> 20;
    pde_index = pde_index | 0xFFFFF000; 
    pde_index = pde_index & ~0x3; 
    return (unsigned long*) pde_index;
}

unsigned long * PageTable::PTE_address(unsigned long addr) {
    unsigned long pte_index = addr >> 10; 
    pte_index = pte_index | 0xFFC00000; 
    pte_index = pte_index & ~0x3; 
    return (unsigned long*) pte_index;
}

void PageTable::handle_fault(REGS * _r)
{
	Console::puts("Handling page fault...");
	unsigned int error_code = _r->err_code;

	// Check for protection fault
	if (error_code & 1) {
	    Console::puts("Detected Protection Fault - not due to a missing page.");
	    return; 
	}
	// Fetch the address causing the page fault
	unsigned long fault_addr = read_cr2();

	bool is_pool_page_valid = false;
	int valid_pools_count = 0;

	// Search through VM pools to validate the faulting address
	for (int i = 0; i < 512; i++) {
	    if (vm[i] != nullptr) { // Check if VM pool slot is used
		if (vm[i]->is_legitimate(fault_addr)) {
		    is_pool_page_valid = true; // Valid pool page found
		    break;
		} else {
		    ++valid_pools_count;
		}
	    }
	}

	// Handling cases with no associated valid pool
	if (!is_pool_page_valid && valid_pools_count > 0) {
	    Console::puts("Address does not belong to a valid pool.");
	    assert(false); 
	}

	// Obtain PDE and PTE pointers for the fault address
	unsigned long *pde = PDE_address(fault_addr);
	unsigned long *pte = PTE_address(fault_addr);

	// Setup PDE and PTE if not already present
	if (*pde & 1) {
	    // If PDE is present, setup PTE
	    *pte = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
	} else {
	    // Setup both PDE and PTE if PDE is absent
	    *pde = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
	    *pte = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
	}
    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
if(pool_number == 512){
        Console::puts("All VM Pools are in use.");
        assert(false);
    }
    vm[pool_number] = _vm_pool;
    pool_number++;
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
	unsigned long* pte_address = PTE_address(_page_no);

	if (*pte_address & 1) {
    	process_mem_pool->release_frames(*pte_address >> 12);
	}
	*pte_address = 0x02;
	write_cr3((unsigned long)page_directory);
	Console::puts("freed page\n");
}
