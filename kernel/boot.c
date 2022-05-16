#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "IDT.h"
#include "assembly.h"
#include "char.h"
#include "elf.h"
#include "gdt.h"
#include "kprint.h"
#include "lock.h"
#include "mem.h"
#include "pic.h"
#include "posix.h"
#include "stivale2.h"
#include "util.h"
#include "cpu.h"
#include "letter_counter.h"

// https://stackoverflow.com/questions/865862/printf-the-current-address-in-c-program
#define ADDRESS_HERE()      \
  ({                        \
    void *p;                \
    __asm__("1: mov 1b, %0" \
            : "=r"(p));     \
    p;                      \
  })

lock_t fork_lock = {.num_locks = 1};

// TODO: Stopped just before Re-enabling System Calls
//  need to set up the TSS and maybe do more things after that

// Reserve space for the stack
static uint8_t stack[8192];

// Request that the beginning of memory (address 0x0) be unmapped
static struct stivale2_tag unmap_null_hdr_tag = {
    .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID, .next = 0};

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {.identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
            .next = (uintptr_t)&unmap_null_hdr_tag},
    .flags = 0};

static struct stivale2_header_tag_smp smp_tag = {
    .tag = {.identifier = STIVALE2_HEADER_TAG_SMP_ID,
            .next = (uintptr_t)&terminal_hdr_tag},
    .flags = 1};

// Declare the header for the bootloader
__attribute__((section(".stivale2hdr"),
               used)) static struct stivale2_header stivale_hdr = {
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

void term_setup(struct stivale2_struct *hdr)
{
  // Look for a terminal tag
  struct stivale2_struct_tag_terminal *tag =
      find_tag(hdr, STIVALE2_STRUCT_TAG_TERMINAL_ID);

  // Make sure we find a terminal tag
  if (tag == NULL)
    halt();

  // Save the term_write function pointer
  // set_term_write((term_write_t)tag->term_write);
}

// int lock_int = 1;

// lock_t our_lock = {.num_locks = 1};

// void func() {
//   lock(&our_lock);
//   kprintf("hello world\n");
//   kprintf("%d\n", our_lock.num_locks);
//   unlock(&our_lock);
//   halt();
// }

// void func2() { halt(); }

// int available_cpus[] = {false, true, true, true};

// int fork() {
//   kprintf("address here %p\n", ADDRESS_HERE());
//   for (int i = 0; i < 4; i++) {
//     kprintf("here\n");
//     if (available_cpus[i]) {
//       kprintf("here1, %d\n", i);
//       available_cpus[i] = false;
//       smp->smp_info[i].goto_address = (uint64_t)(ADDRESS_HERE());
//       return i;
//     }
//   }
//   return -1;
// }

void printer() {
  kprintf("kprint.hhhhhhhhh\n");
  sleep_cpu();
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

<<<<<<< HEAD
  setup_letter_count();
  // letter_count();
  // print_tally();

  // Initialize the stacks for each cpu
  init_cpus(hdr);

  kprintf("here\n");
  kprintf("smp: %p\n", smp);
  // word_count_main(int argc, char **argv);
  for (int i = 1; i < smp->cpu_count; i++)
  {
    kprintf("goto address of %d: %p\n", i, smp->smp_info[i].goto_address);
    smp->smp_info[i].goto_address = letter_count;
  }

  print_tally();

  /* kprintf("%d\n", start_other_core(func)); */
  /* int a = fork(); */
  /* kprintf("here a: %d \n", a); */
  /* halt(); */

  /* for (int i = 1; i < smp->cpu_count; i++) { */
  /*   smp->smp_info[i].goto_address = (uint64_t)(func); */
  /* } */

  // Get information about the modules we've asked the bootloader to load
  struct stivale2_struct_tag_modules *modules =
      find_tag(hdr, STIVALE2_STRUCT_TAG_MODULES_ID);
  // Save information about the modules to be accessed later when we make an
  // exec system call
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
=======
  // setup_letter_count();
  // letter_count();

  // Initialize the stacks for each cpu
  init_cpus(find_tag(hdr, STIVALE2_STRUCT_TAG_SMP_ID));

  int cpu_id1 = set_cpu_task(printer);
  int cpu_id2 = set_cpu_task(printer);

  kprintf("in the main thread after setting\n");

  wait_for_cpu(cpu_id1);
  wait_for_cpu(cpu_id2);
  
  kprintf("homie dis core finished waitin\n");

  // // Get information about the modules we've asked the bootloader to load
  // struct stivale2_struct_tag_modules *modules =find_tag(hdr, STIVALE2_STRUCT_TAG_MODULES_ID);
  // // Save information about the modules to be accessed later when we make an
  // // exec system call
  // module_setup(modules);

  // // kprintf("number of tags: %d\n", smp->cpu_count);
  // // Launch the init program
  // for (int i = 0; i < modules->module_count; i++) {
  //   if (!strcmp(modules->modules[i].string, "init")) {
  //     run_program(modules->modules[i].begin);
  //   }
  // }
>>>>>>> 9ba3a7c20167897d64a6b4997492f3bc6ba36abe

  halt();
}
