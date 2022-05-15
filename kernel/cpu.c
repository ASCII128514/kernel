#include "cpu.h"

struct stivale2_struct_tag_smp* smp;

void init_cpus(struct stivale2_struct_tag_smp *param_smp) {
  // set tag with multi-core stuff
  smp = param_smp;
  // For each cpu initialize the stack
  for (int i = 0; i < smp->cpu_count; i++) {
    uintptr_t cpu_stack = 0x70000000000 + 8 * PAGE_SIZE * i;
    size_t user_stack_size = 8 * PAGE_SIZE;

    // Map the user-mode-stack
    for (uintptr_t p = cpu_stack; p < cpu_stack + user_stack_size; p += 0x1000) {
      // Map a page that is user-accessible, writable, but not executable
      vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
    }

    smp->smp_info[i].target_stack = cpu_stack;
  }
}

uint64_t set_cpu_task(void* address) {

    // find an available cpu
    for (int i = 1; i < smp->cpu_count; i++) {
        if (smp->smp_info[i].goto_address == 0) {
            smp->smp_info[i].goto_address = (uintptr_t) address;
            return i;
        }
    }

    return -1;
}

typedef void (*exec_t)();

void sleep_cpu(int cpu_id) {

    smp->smp_info[cpu_id].goto_address = 0;

    while (1) {
        if (smp->smp_info[cpu_id].goto_address != 0){
            break;
        }
    }

    exec_t new_jump_point = (exec_t) (smp->smp_info[cpu_id].goto_address);

    new_jump_point();
}

int wait_for_cpu(int cpu_id) {

    while (1) {
        if (smp->smp_info[cpu_id].goto_address == 0){
            break;
        }
    }

    return 1;
}
