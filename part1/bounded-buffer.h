/*
* Parker Riebs
* 3/8/2016
*
* Creates a bounded buffer for a user defined amount of buffer space,
* producers, and consumers. Fills the buffer with random numbers between
* 0-9. Then consumes the random number. This is repeated until the
* "Time-to-live" variable is up.
*
* CS 441/541: Bounded Buffer (Project 3 Part 1)
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "semaphore_support.h"
#include <ctype.h>

/*****************************
 * Defines
 *****************************/

/*****************************
 * Structures
 *****************************/
typedef int buffer_item;

/*****************************
 * Global Variables
 *****************************/
buffer_item * buffer;
int buffer_size = 10;
int is_debug = 0;
int total_prod = 0;
int total_cons = 0;
semaphore_t sem_printf;
semaphore_t sem_buffer;
int prod_where = 0;
int cons_where = 0;
int stop_signal = 0;

/*****************************
 * Function Declarations
 *****************************/
/*
 *  Adds random numbers in a buffer in interations.
 *
 *  Parameters:
 *   threadid: The producer id of the thread.
 */
void * producer(void *);
/*
 *  Removes random numbers from a buffer in iterations.
 *
 *  Parameters:
 *    threadid: the consumer id of the thread.
 */
void * consumer(void *);

/*
 *  Prints the information in the buffer.
 *
 */
void print_buffer();