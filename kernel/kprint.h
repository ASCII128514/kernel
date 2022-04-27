#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#include "term.h"
#include "lock.h"

/**
 * Mimic the basic functionality of printf. Given a format string and
 * a variable number of additional arguments, print the format string, with the
 * additional arguments inserted, to the terminal.
 * \param format format string. Accepted format specifiers are: %c (character),
 * %s (string), %d (integer, printed in decimal), %x (integer, printed in hexadecimal),
 * and %p (pointer).
 */
void kprintf(const char *format, ...);

