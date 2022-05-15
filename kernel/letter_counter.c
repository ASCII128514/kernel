#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "kprint.h"

#include "letter_counter.h"

char *input_str = "this is our string";
int input_len;
int num_cpu = 3;

int tally[] = {0, 0, 0, 0, 0,
               0, 0, 0, 0, 0,
               0, 0, 0, 0, 0,
               0, 0, 0, 0, 0,
               0, 0, 0, 0, 0, 0};

lock_t tally_locks[26];
lock_t range_lock = {.num_locks = 1};

void setup_letter_count()
{
    for (int i = 0; i < 26; i++)
    {
        tally_locks[i].num_locks = 1;
    }

    input_len = strlen(input_str);
}

void print_tally()
{
    for (int i = 0; i < 26; i++)
    {
        kprintf("%c: %d | ", 'a' + i, tally[i]);
    }
}

int cpu_count = 0;
void letter_count()
{
    int x = input_len / num_cpu;

    lock(&range_lock);
    int start = x * cpu_count;
    int end = cpu_count == 2 ? input_len : start + x;
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
}