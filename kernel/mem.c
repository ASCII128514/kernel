#include "mem.h"
#include "stivale2.h"
struct stivale2_struct_tag_smp *smp;

node_t *freelist = NULL;

// Translate a physical address to a virtual one
intptr_t ptov(intptr_t addr) { return addr + virtual_offset; }

// Invalidate a TLB entry to get rid of cached address transalations which are
// no longer accurate
void invalidate_tlb(uintptr_t virtual_address) {
  __asm__("invlpg (%0)" ::"r"(virtual_address) : "memory");
}

/**
 * Break up a large piece of memory and add it to the free list
 * \param base   the physical start of the usable memory
 * \param length length of the usable memory in bytes
 */
void add_memory(uint64_t base, uint64_t length) {
  // Round the starting address up to the nearest multiple of a page
  if (base % PAGE_SIZE) {
    length -= (PAGE_SIZE - (base % PAGE_SIZE));
    base += (PAGE_SIZE - (base % PAGE_SIZE));
  }

  // Iterate through the memory given, adding it to the free list in chunks of
  // PAGE_SIZE bytes
  node_t *prev = freelist;
  for (int i = 0; i < length / PAGE_SIZE; i++) {
    node_t *new_node = (node_t *)(uintptr_t)(base + PAGE_SIZE * i);
    new_node->serial_number = SERIAL_NUMBER;
    new_node->next = prev;
    prev = new_node;
  }
  freelist = prev;
}

/**
 * Enables write protection and initializes the freelist, adding to it all
 * memory reported as usable by the bootloader \param virtual  the stivale2
 * struct tag which gives the beginning of the HHDM \param physical the stivale2
 * memory map struct tag, which reports the memory map the memory map built by
 * the bootloader
 */
void freelist_init(struct stivale2_struct_tag_hhdm *virtual,
                   struct stivale2_struct_tag_memmap *physical) {
  // Store the start of the HHDM
  virtual_offset = virtual->addr;

  // Enable write protection
  uint64_t cr0 = read_cr0();
  cr0 |= 0x10000;
  write_cr0(cr0);

  // Loop through physical memory entries, looking for usable memory
  for (int i = 0; i < physical->entries; i++) {
    if (physical->memmap[i].type == 1) {

      // Add usable memory to the freelist
      add_memory(physical->memmap[i].base, physical->memmap[i].length);
    }
  }
}

/**
 * Prints the usable memory, as reported by the bootloader.
 * \param virtual  the stivale2 struct tag which gives the beginning of the HHDM
 * \param physical the stivale2 memory map struct tag, which reports the memory
 * map the memory map built by the bootloader
 */
void get_usable_memory(struct stivale2_struct_tag_hhdm *virtual,
                       struct stivale2_struct_tag_memmap *physical) {
  // Store the start of the HHDM
  virtual_offset = virtual->addr;

  // Loop through physical memory entries, looking for usable memory
  for (int i = 0; i < physical->entries; i++) {
    if (physical->memmap[i].type == 1) {

      // We found usable memory! Print its physical and virtual location
      kprintf("0x%x-0x%x mapped at 0x%x-0x%x\n", physical->memmap[i].base,
              physical->memmap[i].base + physical->memmap[i].length,
              physical->memmap[i].base + virtual_offset,
              physical->memmap[i].base + physical->memmap[i].length +
                  virtual_offset);
    }
  }
}

/**
 * Check the permissions of a page table entry
 * \param table the page table entry
 * \returns 1 if the entry is not present
 *          0 if the entry is present
 */
uint8_t check_permission(page_table_entry_t *table) {

  if (!table->present) {
    kprintf("        not present\n");
    return 1;
  }
  kprintf("        ");
  if (table->kernel) {
    kprintf("user ");
  } else {
    kprintf("kernel ");
  }

  if (table->writable) {
    kprintf("writable ");
  }

  if (!table->no_execute) {
    kprintf("executable");
  }

  kprintf(" -> %p\n", table->physical_addr << 12);

  return 0;
};

/**
 * Translate a virtual address to its mapped physical address
 *
 * \param address     The virtual address to translate
 */
void translate(uintptr_t page_table, void *address) {
  // split the address into 4 level page table

  uintptr_t temp2 = (uintptr_t)address;
  uint16_t offset = temp2 & 0x7ff;
  uint16_t level1 = (temp2 >> 12) & 0x1ff;
  uint16_t level2 = (temp2 >> 21) & 0x1ff;
  uint16_t level3 = (temp2 >> 30) & 0x1ff;
  uint16_t level4 = (temp2 >> 39) & 0x1ff;

  kprintf("Translating: %p\n", address);

  page_table_entry_t *level4_table =
      ((page_table_entry_t *)(page_table + virtual_offset)) + level4;
  kprintf("    Level 4 (index %d of %p)\n", level4, page_table);
  if (check_permission(level4_table)) {
    return;
  }

  uintptr_t level3_start_of_the_page = (level4_table->physical_addr) << 12;
  page_table_entry_t *level3_table =
      ((page_table_entry_t *)(level3_start_of_the_page + virtual_offset)) +
      level3;
  kprintf("    Level 3 (index %d of %p)\n", level3, level3_start_of_the_page);
  if (check_permission(level3_table)) {
    return;
  }

  uintptr_t level2_start_of_the_page = (level3_table->physical_addr) << 12;
  page_table_entry_t *level2_table =
      ((page_table_entry_t *)(level2_start_of_the_page + virtual_offset)) +
      level2;
  kprintf("    Level 2 (index %d of 0x%x)\n", level2, level2_start_of_the_page);
  if (check_permission(level2_table)) {
    return;
  }

  uintptr_t level1_start_of_the_page = level2_table->physical_addr << 12;
  page_table_entry_t *level1_table =
      ((page_table_entry_t *)(level1_start_of_the_page + virtual_offset)) +
      level1;
  kprintf("    Level 1 (index %d of %p)\n", level1, level1_start_of_the_page);
  if (check_permission(level1_table)) {
    return;
  }

  // uint64_t temp = (level1_table[level1]).physical_addr + addr->offset;
  uint64_t physical_addr = (level1_table->physical_addr << 12) + offset;

  kprintf("%p maps to 0x%x\n", address, physical_addr);
}

/**
 * Allocate a page of physical memory.
 * \returns the physical address of the allocated physical memory or 0 on error.
 */
uintptr_t pmem_alloc() {
  node_t *temp = freelist;
  temp = (node_t *)ptov((intptr_t)temp);
  if (temp) {
    freelist = temp->next;
    temp->serial_number = 0;
    return ((uintptr_t)temp) - virtual_offset;
  }
  return 0;
}

/**
 * Free a page of physical memory.
 * \param p is the physical address of the page to free, which must be
 * page-aligned.
 */
void pmem_free(uintptr_t p) {
  if (!(p % PAGE_SIZE)) {
    return;
  }
  node_t *temp = (node_t *)(p + virtual_offset);
  if (temp->serial_number != SERIAL_NUMBER) {
    temp->serial_number = SERIAL_NUMBER;
    temp->next = freelist;
    freelist = (node_t *)(((uintptr_t)temp) - virtual_offset);
  }
  return;
}

/**
 * Add an entry to a page table, and allocate the corresponding page
 * \param table      The page table entry to populate
 * \param user       Should the page be user-accessible?
 * \param writable   Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if no page was allocated, false otherwise
 */
bool malloc_page(page_table_entry_t *table, bool user, bool writable,
                 bool executable) {

  // Get a page for the new entry
  uintptr_t new_page = pmem_alloc();
  if (new_page == 0) {
    // pmem_alloc failed
    return 1;
  }
  // Mark the page as present and fill in the other relevant fields
  table->present = 1;
  table->physical_addr = new_page >> 12;
  table->kernel = user;
  table->writable = writable;
  table->no_execute = !executable;

  // Fill in the new page with zeroes
  new_page += virtual_offset;
  memset((void *)new_page, 0, PAGE_SIZE);

  return 0;
}

/**
 * Map a single page of memory into a virtual address space.
 * \param root       The physical address of the top-level page table structure
 * \param address    The virtual address to map into the address space, must be
 * page-aligned \param user       Should the page be user-accessible? \param
 * writable   Should the page be writable? \param executable Should the page be
 * executable? \returns true if the mapping succeeded, or false if there was an
 * error
 */
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable,
            bool executable) {

  // Break the virtual address into the page table addresses
  uintptr_t temp2 = (uintptr_t)address;
  uint16_t level1 = (temp2 >> 12) & 0x1ff;
  uint16_t level2 = (temp2 >> 21) & 0x1ff;
  uint16_t level3 = (temp2 >> 30) & 0x1ff;
  uint16_t level4 = (temp2 >> 39) & 0x1ff;

  // Traverse the page tables, mapping new pages as necessary
  page_table_entry_t *level4_table =
      ((page_table_entry_t *)(root + virtual_offset)) + level4;
  if (!level4_table->present) {
    if (malloc_page(level4_table, 1, 1, 1)) {
      return false;
    }
  }
  uintptr_t level3_start_of_the_page = (level4_table->physical_addr) << 12;
  page_table_entry_t *level3_table =
      ((page_table_entry_t *)(level3_start_of_the_page + virtual_offset)) +
      level3;
  if (!level3_table->present) {
    if (malloc_page(level3_table, 1, 1, 1)) {
      return false;
    }
  }

  uintptr_t level2_start_of_the_page = (level3_table->physical_addr) << 12;
  page_table_entry_t *level2_table =
      ((page_table_entry_t *)(level2_start_of_the_page + virtual_offset)) +
      level2;
  if (!level2_table->present) {
    if (malloc_page(level2_table, 1, 1, 1)) {
      return false;
    }
  }

  uintptr_t level1_start_of_the_page = level2_table->physical_addr << 12;
  page_table_entry_t *level1_table =
      ((page_table_entry_t *)(level1_start_of_the_page + virtual_offset)) +
      level1;
  if (!level1_table->present) {
    if (malloc_page(level1_table, user, writable, executable)) {
      return false;
    }
  } else {
    // There was already a page mapped at that address
    return false;
  }

  invalidate_tlb(address);
  // Page mapped successfully!
  return true;
}

bool free_page_table(uintptr_t page_head) {
  page_table_entry_t *table = (page_table_entry_t *)page_head;
  for (int i = 0; i < PAGE_SIZE / sizeof(page_table_entry_t); i++) {
    if ((table + i)->present) {
      return false;
    }
  }
  return true;
}

/**
 * Unmap a page from a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to unmap from the address space
 * \returns true if successful, or false if anything goes wrong
 */
bool vm_unmap(uintptr_t root, uintptr_t address) {
  // Break the virtual address into the page table addresses
  uintptr_t temp2 = (uintptr_t)address;
  uint16_t level1 = (temp2 >> 12) & 0x1ff;
  uint16_t level2 = (temp2 >> 21) & 0x1ff;
  uint16_t level3 = (temp2 >> 30) & 0x1ff;
  uint16_t level4 = (temp2 >> 39) & 0x1ff;

  // Traverse the page tables, mapping new pages as necessary
  page_table_entry_t *level4_table =
      ((page_table_entry_t *)(root + virtual_offset)) + level4;
  if (!level4_table->present) {
    return false;
  }

  uintptr_t level3_start_of_the_page = (level4_table->physical_addr) << 12;
  page_table_entry_t *level3_table =
      ((page_table_entry_t *)(level3_start_of_the_page + virtual_offset)) +
      level3;
  if (!level3_table->present) {
    return false;
  }

  uintptr_t level2_start_of_the_page = (level3_table->physical_addr) << 12;
  page_table_entry_t *level2_table =
      ((page_table_entry_t *)(level2_start_of_the_page + virtual_offset)) +
      level2;
  if (!level2_table->present) {
    return false;
  }

  uintptr_t level1_start_of_the_page = level2_table->physical_addr << 12;
  page_table_entry_t *level1_table =
      ((page_table_entry_t *)(level1_start_of_the_page + virtual_offset)) +
      level1;
  if (!level1_table->present) {
    return false;
  }

  level1_table->present = 0;

  pmem_free(level1_table->physical_addr << 12);

  if (free_page_table(level1_start_of_the_page)) {
    level2_table->present = 0;
    pmem_free(level2_table->physical_addr << 12);
  }

  if (free_page_table(level2_start_of_the_page)) {
    level3_table->present = 0;
    pmem_free(level3_table->physical_addr << 12);
  }

  if (free_page_table(level3_start_of_the_page)) {
    level4_table->present = 0;
    pmem_free(level4_table->physical_addr << 12);
  }

  invalidate_tlb(address);

  return true;
}

/**
 * Change the protections for a page in a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to update
 * \param user Should the page be user-accessible or kernel only?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if successful, or false if anything goes wrong (e.g. page is
 * not mapped)
 */
bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable,
                bool executable) {
  // Break the virtual address into the page table addresses
  uintptr_t temp2 = (uintptr_t)address;
  uint16_t level1 = (temp2 >> 12) & 0x1ff;
  uint16_t level2 = (temp2 >> 21) & 0x1ff;
  uint16_t level3 = (temp2 >> 30) & 0x1ff;
  uint16_t level4 = (temp2 >> 39) & 0x1ff;

  // Traverse the page tables, mapping new pages as necessary
  page_table_entry_t *level4_table =
      ((page_table_entry_t *)(root + virtual_offset)) + level4;
  if (!level4_table->present) {
    return false;
  }

  uintptr_t level3_start_of_the_page = (level4_table->physical_addr) << 12;
  page_table_entry_t *level3_table =
      ((page_table_entry_t *)(level3_start_of_the_page + virtual_offset)) +
      level3;
  if (!level3_table->present) {
    return false;
  }

  uintptr_t level2_start_of_the_page = (level3_table->physical_addr) << 12;
  page_table_entry_t *level2_table =
      ((page_table_entry_t *)(level2_start_of_the_page + virtual_offset)) +
      level2;
  if (!level2_table->present) {
    return false;
  }

  uintptr_t level1_start_of_the_page = level2_table->physical_addr << 12;
  page_table_entry_t *level1_table =
      ((page_table_entry_t *)(level1_start_of_the_page + virtual_offset)) +
      level1;
  if (!level1_table->present) {
    return false;
  }

  level1_table->kernel = user;
  level1_table->writable = writable;
  level1_table->no_execute = !executable;
  invalidate_tlb(address);

  return true;
}

// Unmap everything in the lower half of an address space with level 4 page
// table at address root
void unmap_lower_half(uintptr_t root) {
  // We can reclaim memory used to hold page tables, but NOT the mapped pages
  page_table_entry_t *l4_table = (page_table_entry_t *)ptov(root);
  for (size_t l4_index = 0; l4_index < 256; l4_index++) {

    // Does this entry point to a level 3 table?
    if (l4_table[l4_index].present) {

      // Yes. Mark the entry as not present in the level 4 table
      l4_table[l4_index].present = false;

      // Now loop over the level 3 table
      page_table_entry_t *l3_table =
          (page_table_entry_t *)ptov(l4_table[l4_index].physical_addr << 12);
      for (size_t l3_index = 0; l3_index < 512; l3_index++) {

        // Does this entry point to a level 2 table?
        if (l3_table[l3_index].present && !l3_table[l3_index].page_size) {

          // Yes. Loop over the level 2 table
          page_table_entry_t *l2_table = (page_table_entry_t *)ptov(
              l3_table[l3_index].physical_addr << 12);
          for (size_t l2_index = 0; l2_index < 512; l2_index++) {

            // Does this entry point to a level 1 table?
            if (l2_table[l2_index].present && !l2_table[l2_index].page_size) {

              // Yes. Free the physical page the holds the level 1 table
              pmem_free(l2_table[l2_index].physical_addr << 12);
            }
          }

          // Free the physical page that held the level 2 table
          pmem_free(l3_table[l3_index].physical_addr << 12);
        }
      }

      // Free the physical page that held the level 3 table
      pmem_free(l4_table[l4_index].physical_addr << 12);
    }
  }

  // Reload CR3 to flush any cached address translations
  write_cr3(read_cr3());
}

// Find a tag with a given ID
void *find_tag(struct stivale2_struct *hdr, uint64_t id) {
  // Start at the first tag
  struct stivale2_tag *current = (struct stivale2_tag *)hdr->tags;

  // Loop as long as there are more tags to examine
  while (current != NULL) {
    // Does the current tag match?
    if (current->identifier == id) {
      return current;
    }

    // Move to the next tag
    current = (struct stivale2_tag *)current->next;
  }

  // No matching tag found
  return NULL;
}
void init_cpus(struct stivale2_struct *hdr) {
  // Find tag with multi-core stuff
  smp = find_tag(hdr, STIVALE2_STRUCT_TAG_SMP_ID);
  // For each cpu initialize the stack
  for (int i = 0; i < smp->cpu_count; i++) {
    uintptr_t cpu_stack = 0x70000000000 + 8 * PAGE_SIZE * i;
    size_t user_stack_size = 8 * PAGE_SIZE;

    // Map the user-mode-stack
    for (uintptr_t p = cpu_stack; p < cpu_stack + user_stack_size;
         p += 0x1000) {
      // Map a page that is user-accessible, writable, but not executable
      vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
    }

    smp->smp_info[i].target_stack = cpu_stack;
  }
}
