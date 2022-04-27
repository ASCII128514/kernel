#pragma once
#include <stdbool.h>

typedef struct lock_struct
{
  volatile int num_locks;
} lock_t;

/**
 * This function will lock the given lock, and only one process can return.
 *
 * \param lock  the address of a lock
 * \returns void
 */
void lock(lock_t *lock);

/**
 * This function will unlock the given lock
 *
 * \param lock  the address of a lock
 * \returns void
 */
void unlock(lock_t *lock);
