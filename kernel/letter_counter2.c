#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "kprint.h"
#include "cpu.h"

#include "letter_counter2.h"

#define NUM_CPU 3

int input_len;
int cpu_count;
int tally[26];

lock_t tally_locks[26];
lock_t range_lock = {.num_locks = 1};

void setup_letter_count()
{
    kprintf("Input string:\n%s\n", input_str);

    input_len = strlen(input_str);
    cpu_count = 0;

    for (int i = 0; i < 26; i++)
    {
        tally[i] = 0;
        tally_locks[i].num_locks = 1;
    }
}

void print_tally()
{
    for (int i = 0; i < 26; i++)
    {

        kprintf("%c: %d | ", 'a' + i, tally[i]);

        if (i % 4 == 3)
        {
            kprintf("\n");
        }
    }
    kprintf("\n");
}

void letter_count()
{
    int x = input_len / NUM_CPU;

    lock(&range_lock);
    int start = x * cpu_count;
    int end = cpu_count == NUM_CPU - 1 ? input_len : start + x;
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

    sleep_cpu();
}