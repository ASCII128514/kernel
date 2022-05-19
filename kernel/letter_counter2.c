#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "kprint.h"
#include "cpu.h"

#include "letter_counter2.h"

// Defines the number of CPUs to run letter_count()
// Used to correctly divide the work among the CPUs
// This really just defines how many times letter_count()
// needs to run to count every letter, so it can be ran
// on one CPU just the defined number of times
#define NUM_CPU 3

// Tracks the string to be counted
char *input_str;
int input_len;

// Tracks the number of letters counted so far
int tally[26];
lock_t tally_locks[26];

// Used to correctly divide the work among the CPUs
int cpu_count;
lock_t range_lock = {.num_locks = 1};

// Initializes all the variables and locks needed to run letter_count()
// Takes the string that will have its letters counted.
// Must rerun to count a new string or recount a string
void setup_letter_count(char *str)
{
    input_str = str;

    kprintf("Input string:\n%s\n", input_str);

    input_len = strlen(input_str);
    cpu_count = 0;

    for (int i = 0; i < 26; i++)
    {
        tally[i] = 0;
        tally_locks[i].num_locks = 1;
    }
}

// Prints out the number of occurences of each letter in the string
// given as input to setup_letter_count()
// Must call letter_count() NUM_CUP times after calling setup_letter_count()
// to get the correct output
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

// Tallies the letters in the string given as input to setup_letter_count()
// Each call to letter_count() tallies (input_len/NUM_CPU) letters of the
// input string
// Multi-thread safe
// Cannot be called on main CPU because it calls sleep_cpu() at the end
void letter_count()
{
    int num_letters = input_len / NUM_CPU;

    // Sets the range of letters to be tallied
    lock(&range_lock);
    int start = num_letters * cpu_count;
    int end = cpu_count == NUM_CPU - 1 ? input_len : start + num_letters;
    cpu_count++;
    unlock(&range_lock);

    // Performs letter tallying
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