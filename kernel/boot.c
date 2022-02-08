#include <stdint.h>
#include <stddef.h>

#include "stivale2.h"
#include "util.h"
#include "kprint.h"
#include "IDT.h"
#include "pic.h"


// Reserve space for the stack
static uint8_t stack[8192];

// Request that the beginning of memory (address 0x0) be unmapped
static struct stivale2_tag unmap_null_hdr_tag = {
  .identifier = STIVALE2_HEADER_TAG_UNMAP_NULL_ID,
  .next = 0
};

// Request a terminal from the bootloader
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        .next = (uintptr_t)&unmap_null_hdr_tag},
    .flags = 0};


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
    .tags = (uintptr_t)&terminal_hdr_tag};

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
  set_term_write((term_write_t)tag->term_write);
}


void _start(struct stivale2_struct *hdr)
{
  // We've booted! Let's start processing tags passed to use from the bootloader
  term_setup(hdr);
  idt_setup();
  pic_init();
  pic_unmask_irq(1);

  // Find the start of physical memory
  struct stivale2_struct_tag_hhdm *physicalmem = find_tag(hdr, STIVALE2_STRUCT_TAG_HHDM_ID);

  // Get information about virtual memory
  struct stivale2_struct_tag_memmap *vir = find_tag(hdr, STIVALE2_STRUCT_TAG_MEMMAP_ID);

  // Loop through virtual memory entries, looking for usable memory
  for (int i = 0; i < vir->entries; i++) {
    if (vir->memmap[i].type == 1) {
      // We found usable memory! Print its physical and virtual location
      kprintf("0x%d-0x%d mapped at 0x%d-0x%d\n", vir->memmap[i].base,
              vir->memmap[i].base + vir->memmap[i].length, vir->memmap[i].base + physicalmem->addr,
              vir->memmap[i].base + vir->memmap[i].length + physicalmem->addr);
    }
  }

  // We're done, just hang...
  halt();
}
