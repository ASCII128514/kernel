#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <syscall.h>

#include "char.h"
#include "elf.h"
#include "kprint.h"
#include "mem.h"
#include "stivale2.h"
#include <syscall.h>


#define BACKSPACE 8

extern void syscall_entry();

// Place to store the modules loaded by the bootloader
static struct stivale2_struct_tag_modules *modules_list;

/**
 * Write to size characters from buffer to fd
 * \param fd     the location to write to. Must be 1 (stdout) or 2 (stderr)
 * \param buffer string of characters to write
 * \param size   number of characters to be written
 * \return -1 on failure, number of characters written to fd on success
 */
int64_t sys_write(uint64_t fd, intptr_t buffer, uint64_t size);

/**
 * Mimics functionality of C standard library read function.
 * Reads size bytes of data from the location referenced by fd
 * into buffer.
 * \param fd     the location to read from. Must be 0 (standard input)
 * \param buffer location to store bytes read from filedes
 * \param size   number of bytes to read
 * \returns the number of bytes read, or -1 on error
 */
int64_t sys_read(uint64_t fd, intptr_t buffer, uint64_t size);

/**
 * A pared-down version of the C standard library mmap.
 * Maps pages of memory starting at the beginning of the page containing addr
 * and continuing for at most len bytes. Pages are mapped as user-readable,
 * writable, and executable. \param addr   an address on the first page to be
 * mapped \param len    number of bytes to map \param prot   unused \param flags
 * unused \param fd     unused \param offset unused
 */
int64_t sys_mmap(void *addr, size_t len, int prot, int flags, int fd,
                 size_t offset);

/**
 * Load and execute a program specified by file.
 * \param file the program to execute. Must be a null-terminated string
 * \param argv unused
 * \returns -1 if no module matching file was found.
 *    Otherwise, an executable will be run, and so this function should not
 * return.
 */
int64_t sys_exec(char *file, char *argv[]);

/**
 * Clean up after an executable has finished running, and launch the init
 * program. \param status unused \returns status, to match the C standard
 * library exit() system call signature
 */
int64_t sys_exit(int status);

int64_t sys_start_othercore(uintptr_t address);

// Save information about the modules loaded by the bootloader in a global
// variable
void module_setup(struct stivale2_struct_tag_modules *modules);
