#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <syscall.h>
#include "lock.h"

#define STDOUT 1
#define STDIN 0

/**
 * Mimics functionality of C standard library read function.
 * Reads nbyte bytes of data from the location referenced by fildes
 * into the buffer pointed to by buf.
 * \param filedes the location to read from. Must be 0 (standard input)
 * \param buf     array to store bytes read from filedes
 * \param nbyte   number of bytes to read
 * \returns the number of bytes read, or -1 on error
 */
size_t read(int fildes, void *buf, size_t nbyte);

/**
 * Mimics functionality of C standard library write function.
 * Write a specified number of bytes from a buffer to a given file descriptor.
 * \param filedes the file to write to.
 *    Note that right now, filedes must be 1 or 2 (stdout or stderr, respectively),
 *    but that check happens in the system call itself
 * \param buf     array of values to write to the location specified by filedes
 * \param nbyte   number of bytes to write
 * \returns the number of bytes written, or -1 on error
 */
size_t write(int filedes, const void *buf, size_t nbyte);

/**
 * Load and execute a program specified by file.
 * \param file the program to execute. Must be a null-terminated string
 * \param argv unused
 * \returns -1 if no module matching file was found.
 *    Otherwise, an executable will be run, and so this function should not return.
 */
size_t exec(const char *buf, char *arr[]);

/**
 * Clean up after an executable has finished running, and launch the init program.
 * \param status unused
 * \returns status, to match the C standard library exit() system call signature
 */
size_t exit(int status);

/**
 * Mimics functionality of C standard library getline function.
 * Read from the location specified by filedes until a newline character
 * is encountered or a given number of bytes are read.
 * Note that the C standard library function getline allows for dynamic
 * expansion of the buffer *linep, as well as corresponding updates to
 * *linecapp (the size of the buffer). This implementation simply reads
 * a specified number of bytes. It is up to the caller to ensure that
 * *linep is large enough to hold *linecapp bytes.
 * \param linep    pointer to an array to store the values read
 * \param linecapp pointer to the maximum number of bytes to read
 * \param filedes  the file to read from
 *    Note that right now, filedes must be 0 (stdin), but that check happens
 *    in the system call itself
 * \returns the number of bytes read and stored in *linep on success
 *          -1 on error
 */
size_t getline(char* linep, size_t* linecapp, int filedes);

/**
 * Mimic the basic functionality of printf. Given a format string and
 * a variable number of additional arguments, print the format string, with the
 * additional arguments inserted, to the terminal.
 * \param format format string. Accepted format specifiers are: %c (character),
 * %s (string), %d (integer, printed in decimal), %x (integer, printed in hexadecimal),
 * and %p (pointer).
 */
void printf(const char *format, ...);

bool start_other_core(uintptr_t address);
