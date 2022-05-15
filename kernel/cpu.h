#pragma once

#include "stivale2.h"
#include "stddef.h"
#include "stdbool.h"
#include "mem.h"

struct stivale2_struct_tag_smp* get_smp();
void init_cpus(struct stivale2_struct_tag_smp *smp);
uint64_t set_cpu_task(void* address);
void sleep_cpu(int cpu_id);
int wait_for_cpu(int cpu_id);