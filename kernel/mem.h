#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "stivale2.h"
#include "kprint.h"
#include "assembly.h"

#define SERIAL_NUMBER 9123213123
#define PAGE_SIZE 4096

static uint64_t virtual_offset;
extern struct stivale2_struct_tag_smp *smp;

// Format for the entry in a page table
typedef struct page_table {
  uint8_t present : 1;
  uint8_t writable : 1;
  uint8_t kernel : 1;
  uint8_t write_through : 1;
  uint8_t cache_disable : 1;
  uint8_t accessed : 1;
  uint8_t dirty : 1;
  uint8_t page_size : 1;
  uint8_t _unused_0 : 4;
  uint64_t physical_addr : 40;
  uint16_t _unused1 : 11;
  uint8_t no_execute : 1;
} __attribute__((packed)) page_table_entry_t;

// A node in a free list
typedef struct node {
  uint64_t serial_number;
  struct node* next;
} node_t;

/**
 * Enables write protection and initializes the freelist, adding to it all memory
 * reported as usable by the bootloader
 * \param virtual  the stivale2 struct tag which gives the beginning of the HHDM
 * \param physical the stivale2 memory map struct tag, which reports the memory map
 *                 the memory map built by the bootloader
 */
void freelist_init(struct stivale2_struct_tag_hhdm *virtual, struct stivale2_struct_tag_memmap *physical);

/**
 * Prints the usable memory, as reported by the bootloader.
 * \param virtual  the stivale2 struct tag which gives the beginning of the HHDM
 * \param physical the stivale2 memory map struct tag, which reports the memory map
 *                 the memory map built by the bootloader
 */
void get_usable_memory(struct stivale2_struct_tag_hhdm *virtual, struct stivale2_struct_tag_memmap *physical);

/**
 * Translate a virtual address to its mapped physical address
 *
 * \param address     The virtual address to translate
 */
void translate(uintptr_t page_table, void *address);

/**
 * Allocate a page of physical memory.
 * \returns the physical address of the allocated physical memory or 0 on error.
 */
uintptr_t pmem_alloc();

/**
 * Free a page of physical memory.
 * \param p is the physical address of the page to free, which must be page-aligned.
 */
void pmem_free(uintptr_t p);

/**
 * Map a single page of memory into a virtual address space.
 * \param root       The physical address of the top-level page table structure
 * \param address    The virtual address to map into the address space, must be page-aligned
 * \param user       Should the page be user-accessible?
 * \param writable   Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if the mapping succeeded, or false if there was an error
 */
bool vm_map(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable);

/**
 * Unmap a page from a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to unmap from the address space
 * \returns true if successful, or false if anything goes wrong
 */
bool vm_unmap(uintptr_t root, uintptr_t address);

/**
 * Change the protections for a page in a virtual address space
 * \param root The physical address of the top-level page table structure
 * \param address The virtual address to update
 * \param user Should the page be user-accessible or kernel only?
 * \param writable Should the page be writable?
 * \param executable Should the page be executable?
 * \returns true if successful, or false if anything goes wrong (e.g. page is not mapped)
 */
bool vm_protect(uintptr_t root, uintptr_t address, bool user, bool writable, bool executable);

// Translate a physical address to a virtual one
intptr_t ptov(intptr_t addr);

// Unmap everything in the lower half of an address space with level 4 page table at address root
void unmap_lower_half(uintptr_t root);

// Invalidate a TLB entry to get rid of cached address transalations which are no longer accurate
void invalidate_tlb(uintptr_t virtual_address);

void init_cpus(struct stivale2_struct *hdr);

void *find_tag(struct stivale2_struct *hdr, uint64_t id);
