#pragma once
#include <stdint.h>
#include <stddef.h>
#include <syscall.h>

#include "kprint.h"
#include "char.h"

#define SYS_read 0
#define SYS_write 1
#define SYS_mmap 2

#define BACKSPACE 8

extern void syscall_entry();

/**
 * Write to size characters from buffer to fd
 * \param fd     the location to write to. Must be 1 (stdout) or 2 (stderr)
 * \param buffer string of characters to write
 * \param size   number of characters to be written
 * \return -1 on failure, number of characters written to fd on success
 */
int64_t sys_write(uint64_t fd, intptr_t buffer, uint64_t size);

/**
 * Read size characters from fd into buffer
 * \param fd     location to read from. Must be 0 (stdin)
 * \param buffer buffer to store read characters in
 * \param size   number of characters to read
 * \return number of characters read
 */
int64_t sys_read(uint64_t fd, intptr_t buffer, uint64_t size);

int64_t sys_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset);
