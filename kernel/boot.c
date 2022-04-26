#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#include "stivale2.h"
#include "util.h"
#include "kprint.h"
#include "IDT.h"
#include "pic.h"
#include "char.h"
#include "mem.h"
#include "assembly.h"
#include "elf.h"
#include "gdt.h"
#include "posix.h"

// https://stackoverflow.com/questions/865862/printf-the-current-address-in-c-program
#define ADDRESS_HERE() ({ void *p; __asm__("1: mov 1b, %0" : "=r" (p)); p; })

// TODO: Stopped just before Re-enabling System Calls
//  need to set up the TSS and maybe do more things after that

// Reserve space for the stack
static uint8_t stack[8192];

// Request that the beginning of memory (address 0x0) be unmapped
static struct stivale2_tag unmap_null_hdr_tag = {
    .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID,
    .next = 0};

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        .next = (uintptr_t)&unmap_null_hdr_tag},
    .flags = 0};

static struct stivale2_header_tag_smp smp_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_SMP_ID,
        .next = (uintptr_t)&terminal_hdr_tag},
    .flags = 1};

// Declare the header for the bootloader
__attribute__((section(".stivale2hdr"), used)) static struct stivale2_header stivale_hdr = {
    // Use ELF file's default entry point
    .entry_point = 0,

    // Use stack (starting at the top)
    .stack = (uintptr_t)stack + sizeof(stack),

    // Bit 1: request pointers in the higher half
    // Bit 2: enable protected memory ranges (specified in PHDR)
    // Bit 3: virtual kernel mappings (no constraints on physical memory)
    // Bit 4: required
    .flags = 0x1E,

    // First tag struct
    .tags = (uintptr_t)&smp_tag};

// Find a tag with a given ID
void *find_tag(struct stivale2_struct *hdr, uint64_t id)
{
  // Start at the first tag
  struct stivale2_tag *current = (struct stivale2_tag *)hdr->tags;

  // Loop as long as there are more tags to examine
  while (current != NULL)
  {
    // Does the current tag match?
    if (current->identifier == id)
    {
      return current;
    }

    // Move to the next tag
    current = (struct stivale2_tag *)current->next;
  }

  // No matching tag found
  return NULL;
}

void term_setup(struct stivale2_struct *hdr)
{
  // Look for a terminal tag
  struct stivale2_struct_tag_terminal *tag = find_tag(hdr, STIVALE2_STRUCT_TAG_TERMINAL_ID);

  // Make sure we find a terminal tag
  if (tag == NULL)
    halt();

  // Save the term_write function pointer
  // set_term_write((term_write_t)tag->term_write);
}

int lock_int = 1;

typedef struct lock_struct
{
  int num_locks;
} lock_t;
int my_lock = 1;
int expected = 1;

void lock(lock_t *lock)
{
  // __atomic_compare_exchange_n(type * ptr, type * expected, type desired, bool weak, int success_memorder, int failure_memorder) return 1;
  while (__atomic_compare_exchange_n(&my_lock, &expected, 0, false, __ATOMIC_SEQ_CST, 0) == 0)
    ;
}

void unlock(lock_t *lock)
{
  lock->num_locks = 1;
}

lock_t our_lock = {.num_locks = 1};

void func()
{
  lock(&our_lock);
  // kprintf("hello world\n");
  kprintf("%d\n", our_lock.num_locks);
  unlock(&our_lock);
  // int val = 1;
  // if (val)
  // {
  //   val = 0;
  //   void *here = ADDRESS_HERE();
  //   // kprintf("core: %d, address: %p\n", ADDRESS_HERE());
  // }
  // else
  // {
  //   kprintf("ppppppp\n");
  // }
  halt();
}

void func2()
{
  halt();
}

void _start(struct stivale2_struct *hdr)
{
  // We've booted! Let's start processing tags passed to use from the bootloader
  term_setup(hdr);

  // Set up the interrupt descriptor table and global descriptor table
  idt_setup();
  gdt_setup();

  // Find the start of higher half direct map (virtual memory)
  struct stivale2_struct_tag_hhdm *virtual = find_tag(hdr, STIVALE2_STRUCT_TAG_HHDM_ID);

  // Get information about physical memory
  struct stivale2_struct_tag_memmap *physical = find_tag(hdr, STIVALE2_STRUCT_TAG_MEMMAP_ID);

  struct stivale2_struct_tag_smp *smp = find_tag(hdr, STIVALE2_STRUCT_TAG_SMP_ID);

  // Set up the free list and enable write protection
  freelist_init(virtual, physical);

  // Initialize the terminal
  term_init();

  // Set up keyboard interrupts
  pic_init();
  pic_unmask_irq(1);

  // Unmap the lower half of memory
  uintptr_t root = read_cr3() & 0xFFFFFFFFFFFFF000;
  unmap_lower_half(root);

  kprintf("hi: %d\n", our_lock.num_locks);

  for (int i = 0; i < smp->cpu_count; i++)
  {
    uintptr_t cpu_stack = 0x70000000000 + 8 * PAGE_SIZE * i;
    size_t user_stack_size = 8 * PAGE_SIZE;

    // Map the user-mode-stack
    for (uintptr_t p = cpu_stack; p < cpu_stack + user_stack_size; p += 0x1000)
    {
      // Map a page that is user-accessible, writable, but not executable
      vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
    }
    smp->smp_info[i].target_stack = cpu_stack;
    kprintf("%p\n", cpu_stack);
  }

  for (int i = 1; i < smp->cpu_count; i++)
  {
    // if (1)
    // {

    smp->smp_info[i].extra_argument = i;
    smp->smp_info[i].goto_address = (uint64_t)(func);
    // }
    // else if (i == 2)
    // {
    //   // for (int i = 0; i < 0xf0000000;i++){};
    //   // smp->smp_info[i].goto_address = (uint64_t)(func);
    // }
    // else
    // {
    //   smp->smp_info[i].goto_address = (uint64_t)(func2);
    // }
  }

  // Get information about the modules we've asked the bootloader to load
  struct stivale2_struct_tag_modules *modules = find_tag(hdr, STIVALE2_STRUCT_TAG_MODULES_ID);
  // Save information about the modules to be accessed later when we make an exec system call
  module_setup(modules);

  kprintf("number of tags: %d\n", smp->cpu_count);
  // Launch the init program
  for (int i = 0; i < modules->module_count; i++)
  {
    if (!strcmp(modules->modules[i].string, "init"))
    {
      run_program(modules->modules[i].begin);
    }
  }

  halt();
}
