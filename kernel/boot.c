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
// #include "letter_counter.h"
#include "letter_counter2.h"

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

void printer()
{
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

  // Initialize the stacks for each cpu
  init_cpus(find_tag(hdr, STIVALE2_STRUCT_TAG_SMP_ID));

  int cpu_id1;
  int cpu_id2;
  int cpu_id3;

  // ************************************************* <  DEMO 1 > *************************************************
  // Demonstrates that our code and correctly count letters and be ran multiple times in succession

  kprintf("Setting up letter count --------------------\n");
  setup_letter_count("the quick brown fox jumps over the lazy dog");

  cpu_id1 = set_cpu_task(letter_count);
  cpu_id2 = set_cpu_task(letter_count);
  cpu_id3 = set_cpu_task(letter_count);
  wait_for_cpu(cpu_id1);
  wait_for_cpu(cpu_id2);
  wait_for_cpu(cpu_id3);

  kprintf("Finished waiting for all cores\n");
  print_tally();

  kprintf("\nSetting up letter count --------------------\n");
  setup_letter_count("we can change the input sentence.");
  cpu_id1 = set_cpu_task(letter_count);
  cpu_id2 = set_cpu_task(letter_count);
  cpu_id3 = set_cpu_task(letter_count);
  wait_for_cpu(cpu_id1);
  wait_for_cpu(cpu_id2);
  wait_for_cpu(cpu_id3);

  kprintf("Finished waiting for all cores\n");

  print_tally();

  // ************************************************* <  DEMO 2 > *************************************************
  // Demonstrates what happens when not all the expected calls to letter_count are made

  // kprintf("Setting up letter count --------------------\n");
  // setup_letter_count("abcd efgh ijkl ");

  // cpu_id1 = set_cpu_task(letter_count);
  // cpu_id2 = set_cpu_task(letter_count);
  // cpu_id3 = set_cpu_task(letter_count);
  // wait_for_cpu(cpu_id1);
  // wait_for_cpu(cpu_id2);
  // wait_for_cpu(cpu_id3);

  // kprintf("Finished waiting for all cores\n");
  // print_tally();

  // kprintf("\nSetting up letter count --------------------\n");
  // setup_letter_count("abcd efgh ijkl ");

  // cpu_id1 = set_cpu_task(letter_count);

  // cpu_id3 = set_cpu_task(letter_count);
  // wait_for_cpu(cpu_id1);

  // wait_for_cpu(cpu_id3);

  // kprintf("Finished waiting for all cores\n");

  // print_tally();

  // ************************************************* <  DEMO 3 > *************************************************
  // Demonstrates counting a large string

  // kprintf("\nSetting up letter count --------------------\n");
  // setup_letter_count("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  // kprintf("There are 1000 a's\n");
  // cpu_id1 = set_cpu_task(letter_count);
  // cpu_id2 = set_cpu_task(letter_count);
  // cpu_id3 = set_cpu_task(letter_count);
  // wait_for_cpu(cpu_id1);
  // wait_for_cpu(cpu_id2);
  // wait_for_cpu(cpu_id3);
  // kprintf("Finished waiting for all cores\n");

  // print_tally();

  // ************************************************* < DEMO 4 > *************************************************
  // Demonstrates counting a large string without waiting causes issues

  // kprintf("\nSetting up letter count --------------------\n");
  // setup_letter_count("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  // kprintf("There are 1000 a's\n");
  // cpu_id1 = set_cpu_task(letter_count);
  // cpu_id2 = set_cpu_task(letter_count);
  // cpu_id3 = set_cpu_task(letter_count);

  // print_tally();

  // ********************************************** <  END OF DEMOS > **********************************************

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

  halt();
}
