#include "lock.h"

void lock(lock_t *lock)
{
  int expected = 1;
  // __atomic_compare_exchange_n(type * ptr, type * expected, type desired, bool weak, int success_memorder, int failure_memorder) return 1;
  while (__atomic_compare_exchange_n(&(lock->num_locks), &expected, 0, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) == 0)
    expected = 1;
}

void unlock(lock_t *lock)
{
  lock->num_locks = 1;
}
