#pragma once

/**
 * @brief Tallies the letters of the string set in setup_letter_count()
 * @return void
 * @note This function is meant to be passed to child processes because it has sleep_cpu()
 * at the end of the function. If it is passed to the main cpu, the program will just halt
 */
void letter_count();

/**
 * @brief Initializes all of the locks and arrays required to run letter_count()
 * @param str the string whose letters will be tallied
 * @return void
 * @note This function must be ran again before every call to letter_count()
 */
void setup_letter_count(char *str);

/**
 * @brief Prints out the counts for each letter after running letter_count()
 * @return void
 * @note Must run setup_letter_count() and letter_count() before running this function
 */
void print_tally();