#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "kprint.h"

#include "letter_counter.h"

#define NUM_CPU 3

char *input_str = "aaaa aaaa aaaa ";
int input_len;

int tally[] = {0, 0, 0, 0, 0,
               0, 0, 0, 0, 0,
               0, 0, 0, 0, 0,
               0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0};

lock_t tally_locks[26];
lock_t range_lock = {.num_locks = 1};
lock_t print_lock[NUM_CPU];

void setup_letter_count()
{
    for (int i = 0; i < 26; i++)
    {
        tally_locks[i].num_locks = 1;
    }

    for (int i = 0; i < NUM_CPU; i++)
    {
        print_lock[i].num_locks = 0;
    }

    input_len = strlen(input_str);
}

void print_tally()
{
    for (int i = 0; i < NUM_CPU; i++)
    {
        lock(&print_lock[i]);
        unlock(&print_lock[i]);
    }

    for (int i = 0; i < 26; i++)
    {
        kprintf("%c: %d | ", 'a' + i, tally[i]);
    }
}

int cpu_count = 0;
void letter_count()
{
    int x = input_len / NUM_CPU;

    lock(&range_lock);
    int cpu = cpu_count;
    int start = x * cpu_count;
    int end = cpu_count == NUM_CPU - 1 ? input_len : start + x;
    // int end = cpu_count == NUM_CPU - 1 ? 0 : start + x;
    cpu_count++;
    unlock(&range_lock);

    for (int i = start; i < end; i++)
    {
        if ((input_str[i] >= 'a') && (input_str[i] <= 'z'))
        {
            lock(&(tally_locks[input_str[i] - 'a']));
            tally[input_str[i] - 'a']++;
            unlock(&(tally_locks[input_str[i] - 'a']));
        }
    }

    unlock(&print_lock[cpu]);

    while (1)
        ;
}