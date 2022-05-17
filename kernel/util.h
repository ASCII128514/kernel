#pragma once

// Halt the CPU in an infinite loop
__attribute__((no_caller_saved_registers)) static void halt()
{
  while (1) {
    __asm__("hlt");
  }
  return;
}
