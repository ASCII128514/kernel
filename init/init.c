#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mman.h>
#include <syscall.h>
#include <unistd.h>
void test() {
  printf("howdy\n");
}
void func() {
  for (int i = 0; i < 100000; i++) {}; //spin a bit
  printf("hello world\n");
  while (true) {}; // spin forever
}

void _start() {
  printf("hey I'm inside the _start() function :)\n");

  printf("the address of func is %p\n", &func);
  printf("the address of start is %p\n", _start);

  int val = start_other_core(func);

  printf("val is %d\n", val);

  while (1 < 2) {}
  // exit(0);
}
