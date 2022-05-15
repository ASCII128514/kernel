// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <unistd.h>
// #include <string.h>
// #include <fcntl.h>
// #include <sys/mman.h>
// #include <stdbool.h>
// #include <ctype.h>
// #include <pthread.h>

// #define PAGE_SIZE 0x1000
// #define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// /// The number of times we've seen each letter in the input, initially zero
// size_t letter_counts[26] = {0};
// pthread_mutex_t letter_locks[26];
// size_t thread_data_amount;

// // the struct passed along to our threads casted to a void*
// typedef struct thread_params
// {
//   int start_position;
//   char *file_data;
//   bool finisher;
//   off_t file_size;
// } thread_params_t;

// // function index
// void *count_letter_parallel(void *void_params);

// /**
//  * This function should divide up the file_data between the specified number of
//  * threads, then have each thread count the number of occurrences of each letter
//  * in the input data. Counts should be written to the letter_counts array. Make
//  * sure you count only the 26 different letters, treating upper- and lower-case
//  * letters as the same. Skip all other characters.
//  *
//  * \param num_threads   The number of threads your program must use to count
//  *                      letters. Divide work evenly between threads
//  * \param file_data     A pointer to the beginning of the file data array
//  * \param file_size     The number of bytes in the file_data array
//  */
// void count_letters(int num_threads, char *file_data, off_t file_size)
// {

//   // initialize some global variables
//   thread_data_amount = file_size / num_threads;

//   // initialize our locks
//   for (int i = 0; i < 26; i++)
//   {
//     pthread_mutex_init(&letter_locks[i], NULL);
//   }

//   int pos = 0;
//   pthread_t threads[num_threads];
//   thread_params_t *params[num_threads];

//   // create our thread parameters and the threads
//   for (int i = 0; i < num_threads; i++)
//   {
//     params[i] = malloc(sizeof(thread_params_t));
//     params[i]->start_position = pos;
//     params[i]->file_data = file_data;
//     params[i]->file_size = file_size;
//     pos += thread_data_amount;
//     params[i]->finisher = (i == (num_threads - 1)) ? true : false;
//     pthread_create(&(threads[i]), NULL, count_letter_parallel, (void *)params[i]);
//   }

//   // wait for all threads to finish
//   for (int i = 0; i < num_threads; i++)
//   {
//     if (pthread_join(threads[i], NULL))
//     {
//       perror("pthread_join error");
//       exit(2);
//     }
//   }

//   // free our thread parameters
//   for (int i = 0; i < num_threads; i++)
//   {
//     free(params[i]);
//   }
// }

// void *count_letter_parallel(void *void_params)
// {
//   thread_params_t *params = (thread_params_t *)void_params;

//   int i = params->start_position;

//   // go through the threads given chunk
//   while (i < (params->start_position + thread_data_amount))
//   {
//     int ch = params->file_data[i];
//     if (isupper((char)ch) != 0)
//     {
//       ch = tolower(ch);
//     }
//     ch = ch - 'a';
//     if (ch < 0 || ch > 25)
//     {
//       i++;
//       continue;
//     }
//     pthread_mutex_lock(&letter_locks[ch]);
//     letter_counts[ch]++;
//     pthread_mutex_unlock(&letter_locks[ch]);
//     i++;
//   }

//   // if the last thread, make sure to reach the end of the file
//   if (params->finisher)
//   {
//     while (i < params->file_size)
//     {
//       int ch = params->file_data[i];
//       if (isupper((char)ch) != 0)
//       {
//         ch = tolower(ch);
//       }
//       ch = ch - 'a';
//       if (ch < 0 || ch > 25)
//       {
//         i++;
//         continue;
//       }
//       pthread_mutex_lock(&letter_locks[ch]);
//       letter_counts[ch]++;
//       pthread_mutex_unlock(&letter_locks[ch]);
//       i++;
//     }
//   }

//   return NULL;
// }

// /**
//  * Show instructions on how to run the program.
//  * \param program_name  The name of the command to print in the usage info
//  */
// void show_usage(char *program_name)
// {
//   fprintf(stderr, "Usage: %s <N> <input file>\n", program_name);
//   fprintf(stderr, "    where <N> is the number of threads (1, 2, 4, or 8)\n");
//   fprintf(stderr, "    and <input file> is a path to an input text file.\n");
// }

// int word_count_main(int argc, char **argv)
// {
//   // Check parameter count
//   if (argc != 2)
//   {
//     show_usage(argv[0]);
//     exit(1);
//   }

//   // Read thread count
//   int num_threads = atoi(argv[1]);
//   if (num_threads != 1 && num_threads != 2 && num_threads != 4 && num_threads != 8)
//   {
//     fprintf(stderr, "Invalid number of threads: %s\n", argv[1]);
//     show_usage(argv[0]);
//     exit(1);
//   }

//   char * str = "asdasdasdasdasd";
//   // Call the function to count letter frequencies
//   count_letters(num_threads, str, strlen(str));

//   // Print the letter counts
//   for (int i = 0; i < 26; i++)
//   {
//     printf("%c: %lu\n", 'a' + i, letter_counts[i]);
//   }

//   return 0;
// }
