#include "elf.h"

typedef void (*void_function_t)();

/**
 * Run an executable stored the ELF file which begins at virtual address addr
 * \param addr the virtual address of the beginning of the ELF file
 */
void run_program(intptr_t addr) {
  // ELF files are guaranteed to start with a header; cast addr to
  // a structure matching the header exactly
  elfhdr_t *hdr = (elfhdr_t *) addr;

  // Get the virtual address of the program entry point
  intptr_t entry = hdr->e_entry;
  // Get the virtual address of the beginning of the program header table
  phdr_t* phead = (phdr_t *)(hdr->e_phoff + addr);
  // Get the number of entries in the program header table
  int ph_num = hdr->e_phnum;

  // Get the address of the top level page table
  uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;

  // Iterate over the program header table, mapping each segments to its place in virtual memory
  for (int i = 0; i < ph_num; i++) {
    // The current entry (segment)
    phdr_t * h = phead + i;

    // If the segment has type load and its virtual size  is nonzero, map it
    if ((h->p_type == PT_LOAD) && h->p_memsz) {
      void *file_ptr = (void *)(addr + h->p_offset); // Virtual address of the beginning of the segment
      void *v_ptr = (void *)h->p_vaddr; // Virtual address where the segment must be placed
      int executable = h->p_flags & 0x1;
      int writable =( h->p_flags & 0x2) >> 1;
      int readable = (h->p_flags & 0x4) >> 2;

      for (int j = 0; j < ((h->p_vaddr + h->p_memsz - (h->p_vaddr & 0xFFFFFFFFFFFFF000)) / PAGE_SIZE + 1); j++) {
        if (!vm_map(root, (((intptr_t)v_ptr) & 0xFFFFFFFFFFFFF000) + j*PAGE_SIZE, 1, 1, 0))
        {
          kprintf("mem map failed\n");
        };                                   // Map the virtual address we need
      }
      memcpy(v_ptr, file_ptr, h->p_memsz); // Copy the segment to its new place
      for (int j = 0; j < ((h->p_vaddr + h->p_memsz - (h->p_vaddr & 0xFFFFFFFFFFFFF000)) / PAGE_SIZE + 1); j++) {
        if (!vm_protect(root, (((intptr_t)v_ptr) & 0xFFFFFFFFFFFFF000) + j * PAGE_SIZE, readable, writable, executable)) {
          kprintf("protect failed\n");
        }
      }
    }
  }

  // Everything is set up! Now jump to entry point of the function

  // Pick an arbitrary location and size for the user-mode stack
  uintptr_t user_stack = 0x70000000000;
  size_t user_stack_size = 8 * PAGE_SIZE;

  // Map the user-mode-stack
  for(uintptr_t p = user_stack; p < user_stack + user_stack_size; p += 0x1000) {
    // Map a page that is user-accessible, writable, but not executable
    vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
  }

  // And now jump to the entry point
  usermode_entry(USER_DATA_SELECTOR | 0x3,            // User data selector with priv=3
                  user_stack + user_stack_size - 8,   // Stack starts at the high address minus 8 bytes
                  USER_CODE_SELECTOR | 0x3,           // User code selector with priv=3
                  entry);                             // Jump to the entry point specified in the ELF file
  
}

/**
 * Run an executable stored the ELF file which begins at virtual address addr
 * \param addr the virtual address of the beginning of the ELF file
 */
void run_program_kernel(intptr_t addr) {
  // ELF files are guaranteed to start with a header; cast addr to
  // a structure matching the header exactly
  elfhdr_t *hdr = (elfhdr_t *) addr;

  // Get the virtual address of the program entry point
  intptr_t entry = hdr->e_entry;
  // Get the virtual address of the beginning of the program header table
  phdr_t* phead = (phdr_t *)(hdr->e_phoff + addr);
  // Get the number of entries in the program header table
  int ph_num = hdr->e_phnum;

  // Get the address of the top level page table
  uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;

  // Iterate over the program header table, mapping each segments to its place in virtual memory
  for (int i = 0; i < ph_num; i++) {
    // The current entry (segment)
    phdr_t * h = phead + i;

    // If the segment has type load and its virtual size  is nonzero, map it
    if ((h->p_type == PT_LOAD) && h->p_memsz) {
      void *file_ptr = (void *)(addr + h->p_offset); // Virtual address of the beginning of the segment
      void *v_ptr = (void *)h->p_vaddr; // Virtual address where the segment must be placed
      int executable = h->p_flags & 0x1;
      int writable =( h->p_flags & 0x2) >> 1;
      int readable = (h->p_flags & 0x4) >> 2;

      for (int j = 0; j < ((h->p_vaddr + h->p_memsz - (h->p_vaddr & 0xFFFFFFFFFFFFF000)) / PAGE_SIZE + 1); j++) {
        if (!vm_map(root, (((intptr_t)v_ptr) & 0xFFFFFFFFFFFFF000) + j*PAGE_SIZE, 1, 1, 0))
        {
          kprintf("mem map failed\n");
        };                                   // Map the virtual address we need
      }
      memcpy(v_ptr, file_ptr, h->p_memsz); // Copy the segment to its new place
      for (int j = 0; j < ((h->p_vaddr + h->p_memsz - (h->p_vaddr & 0xFFFFFFFFFFFFF000)) / PAGE_SIZE + 1); j++) {
        if (!vm_protect(root, (((intptr_t)v_ptr) & 0xFFFFFFFFFFFFF000) + j * PAGE_SIZE, readable, writable, executable)) {
          kprintf("protect failed\n");
        }
      }
    }
  }

  // Everything is set up! Now jump to entry point of the function

  // Pick an arbitrary location and size for the user-mode stack
  uintptr_t user_stack = 0x70000000000;
  size_t user_stack_size = 8 * PAGE_SIZE;

  // Map the user-mode-stack
  for(uintptr_t p = user_stack; p < user_stack + user_stack_size; p += 0x1000) {
    // Map a page that is user-accessible, writable, but not executable
    vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
  }

  // And now jump to the entry point
  usermode_entry(KERNEL_DATA_SELECTOR | 0x3,            // User data selector with priv=3
                  user_stack + user_stack_size - 8,   // Stack starts at the high address minus 8 bytes
                  KERNEL_CODE_SELECTOR | 0x3,           // User code selector with priv=3
                  entry);                             // Jump to the entry point specified in the ELF file
  
}

