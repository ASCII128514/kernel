#pragma once

#include "stivale2.h"
#include "stddef.h"
#include "stdbool.h"
#include "mem.h"

/**
 * @brief Get the smp object
 * @return the pointer to the smp object
 * @note This function is only available on x86_64
 */
struct stivale2_struct_tag_smp* get_smp();

/**
 * @brief init all the cpus with proper stack
 * @return void
 * @note This function is only available on x86_64
 */
void init_cpus(struct stivale2_struct_tag_smp *smp);

/*
  * @brief set the cpu task a cpu should run, then run the task
  * @param address the address to jump to
  * @return the cpu id
  * @note This function is only available on x86_64
  */
uint64_t set_cpu_task(void* address);

/**
 * @brief send the cpu to sleep and waiting for a new task
 * @return void
 * @note This function is only available on x86_64
 */
void sleep_cpu();

/**
 * @brief wait for the cpu with the specified cpu id.
 * @return the cpu id that this cpu should wait for.
 * @note This function is only available on x86_64
 */
int wait_for_cpu(int cpu_id);
