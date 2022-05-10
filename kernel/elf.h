#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "kprint.h"
#include "assembly.h"
#include "posix.h"
#include "mem.h"
#include "gdt.h"
#include "usermode_entry.h"

// Structures for the ELF header and program header table entries were
// obtained from https://uclibc.org/docs/elf-64-gen.pdf

#define EI_NIDENT 16
#define PT_LOAD 1

// Structure of an ELF file header
typedef struct elfhdr {
  unsigned char e_ident[EI_NIDENT]; /* ELF identification */
  uint16_t e_type;         /* Object file type */
  uint16_t e_machine;      /* Machine type */
  uint32_t e_version;      /* Object file version */
  uint64_t e_entry;        /* Entry point address */
  uint64_t e_phoff;        /* Program header offset */
  uint64_t e_shoff;        /* Section header offset */
  uint32_t e_flags;        /* Processor-specific flags */
  uint16_t e_ehsize;       /* ELF header size */
  uint16_t e_phentsize;    /* Size of program header entry */
  uint16_t e_phnum;        /* Number of program header entries */
  uint16_t e_shentsize;    /* Size of section header entry */
  uint16_t e_shnum;        /* Number of section header entries */
  uint16_t e_shstrndx;     /* Section name string table index */

} __attribute__((packed)) elfhdr_t;

// Structure of an entry in the program header table
typedef struct phdr {
  uint32_t p_type;    /* Type of segment */
  uint32_t p_flags;   /* Segment attributes */
  uint64_t p_offset;  /* Offset in file */
  uint64_t p_vaddr;   /* Virtual address in memory */
  uint64_t p_paddr;   /* Reserved */
  uint64_t p_filesz;  /* Size of segment in file */
  uint64_t p_memsz;   /* Size of segment in memory */
  uint64_t p_align;   /* Alignment of segment */
} __attribute__((packed)) phdr_t;

/**
 * Run an executable stored the ELF file which begins at virtual address addr
 * \param addr the virtual address of the beginning of the ELF file
 */
void run_program(intptr_t addr);

void run_program_kernel(intptr_t addr);

