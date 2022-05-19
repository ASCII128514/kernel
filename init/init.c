#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mman.h>
#include <syscall.h>
#include <unistd.h>

void func() {
  printf("hello world\n");
  for(;;){}
}

void _start() {
  printf("here: %d\n", start_other_core((uint64_t)func));
  // Start the shell
  exec("shell", NULL);
  exit(0);
}
