#include "unistd.h"

lock_t print_lock = {.num_locks = 1};

/**
 * Mimics functionality of C standard library read function.
 * Reads nbyte bytes of data from the location referenced by fildes
 * into the buffer pointed to by buf.
 * \param filedes the location to read from. Must be 0 (standard input)
 * \param buf     array to store bytes read from filedes
 * \param nbyte   number of bytes to read
 * \returns the number of bytes read, or -1 on error
 */
size_t read(int fildes, void *buf, size_t nbyte) {
  return syscall(SYS_read, fildes, (intptr_t)buf, nbyte);
}

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
size_t write(int filedes, const void *buf, size_t nbyte) {
    return syscall(SYS_write, filedes, buf, nbyte);
}

/**
 * Load and execute a program specified by file.
 * \param file the program to execute. Must be a null-terminated string
 * \param argv unused
 * \returns -1 if no module matching file was found.
 *    Otherwise, an executable will be run, and so this function should not return.
 */
size_t exec(const char *buf, char *arr[]) {
  return syscall(SYS_exec, buf, arr);
}

/**
 * Clean up after an executable has finished running and launch the init program.
 * \param status unused
 * \returns status, to match the C standard library exit() system call signature
 */
size_t exit(int status) {
  return syscall(SYS_exit, status);
}

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
size_t getline(char* linep, size_t* linecapp, int filedes) {
  size_t i = 0;
    for(; i < *linecapp; i++) {
        if(read(STDIN, (linep) + i, 1) == -1) {
            return -1;
        } else if((linep)[i] == '\n') {
          return i + 1;
        }
    }
    *linecapp = i;
    return *linecapp;
}


// Recursive helper function for printing numbers in decimal
void print_d_helper(uint64_t value)
{
  if (value != 0)
  {
    print_d_helper(value / 10); //> Recursive call

    uint64_t temp = value % 10 + 48;
    write(STDOUT, &temp, 1); //> Print the least significant digit
  }
}

// Recursive helper function for printing numbers in hexidecimal
void print_x_helper(uint64_t value)
{
  // Hexadecimal digits
  char table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  if (value != 0)
  {
    print_x_helper(value / 16); //> Recursive call

    write(STDOUT, &table[value % 16], 1); //> Print the least significant digit
  }
}

/**
 * Mimic the basic functionality of printf. Given a format string and
 * a variable number of additional arguments, print the format string, with the
 * additional arguments inserted, to the terminal.
 * \param format format string. Accepted format specifiers are: %c (character),
 * %s (string), %d (integer, printed in decimal), %x (integer, printed in hexadecimal),
 * and %p (pointer).
 */
void printf(const char *format, ...) {

  // lock the function
  lock(&print_lock);
  // Start processing variadic arguments
  va_list args;
  va_start(args, format);

  // Loop until we reach the end of the format string
  size_t index = 0;
  uint64_t value;
  char temp_char;
  char* temp_str;
  while (format[index] != '\0') {
    // Is the current charater a '%'?
    if (format[index] == '%') {
      // Yes, print the argument
      index++;
      switch (format[index]) {
      case '%':
        write(STDOUT, &(format[index]), 1);
        break;
      case 'c':
        temp_char = va_arg(args, int);
        write(STDOUT, &temp_char, 1);
        break;
      case 's':
        temp_str = va_arg(args, char *);
        write(STDOUT, temp_str, strlen(temp_str));
        break;
      case 'd':
        value = va_arg(args, uint64_t);
        if (value != 0) {
          print_d_helper(value);
        } else {
          write(STDOUT, "0", 1);
        }
        break;
      case 'x':
        value = va_arg(args, uint64_t);
        if (value != 0) {
          print_x_helper(value);
        } else {
          write(STDOUT, "0", 1);
        }
        break;
      case 'p':
        write(STDOUT, "0x", 2);
        value = va_arg(args, uint64_t);
        if (value != 0) {
          print_x_helper(value);
        } else {
          write(STDOUT, "0", 1);
        }
        break;
      default:
        write(STDOUT, "<not supported>", 15);
      }
    } else {
      // No, just a normal character. Print it.
      write(STDOUT, &format[index], 1);
    }
    index++;
  }

  // Finish handling variadic arguments
  va_end(args);

  // unlock the function
  unlock(&print_lock);
}
