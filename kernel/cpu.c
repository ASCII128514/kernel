#include "cpu.h"
#include "lock.h"

struct stivale2_struct_tag_smp* smp;

lock_t my_lock = {.num_locks = 1};

struct stivale2_struct_tag_smp* get_smp() {
    return smp;
}

void init_cpus(struct stivale2_struct_tag_smp *param_smp) {
  // set tag with multi-core stuff
  smp = param_smp;

  uintptr_t cpu_stack = 0x80000000000;
  size_t cpu_stack_size = 8 * PAGE_SIZE;

  // For each cpu initialize the stack
  for (int i = 0; i < smp->cpu_count; i++) {

    // Map the cpu-stack
    for (uintptr_t p = cpu_stack; p < cpu_stack + cpu_stack_size; p += 0x1000) {
      // Map a page that is user-accessible, writable, but not executable
      vm_map(read_cr3() & 0xFFFFFFFFFFFFF000, p, true, true, false);
    }

    smp->smp_info[i].target_stack = cpu_stack;
    cpu_stack += cpu_stack_size;
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

int indentify_cpu() {

    int x = 0;
    uintptr_t address = (uintptr_t) (&x);

    if (address >= 0x80000000000 && address < 0x80000008000) {
        return 1;
    } else if (address >= 0x80000008000 && address < 0x80000010000) {
        return 2;
    } else if (address >= 0x80000010000 && address < 0x80000018000) {
        return 3;
    }

    return -1;
}

typedef void (*exec_t)();

void sleep_cpu() {

    int cpu_id = indentify_cpu();

    smp->smp_info[cpu_id].goto_address = 0;

    while (1) {
        lock(&my_lock);
        if (smp->smp_info[cpu_id].goto_address != 0){
            unlock(&my_lock);
            break;
        }
        unlock(&my_lock);
    }

    exec_t new_jump_point = (exec_t) (smp->smp_info[cpu_id].goto_address);

    new_jump_point();
}

int wait_for_cpu(int cpu_id) {

    while (1) {
        lock(&my_lock);
        if (smp->smp_info[cpu_id].goto_address == 0){
            unlock(&my_lock);
            break;
        }
        unlock(&my_lock);
    }

    kprintf("I'm done waiting!!!\n");

    return 1;
}
