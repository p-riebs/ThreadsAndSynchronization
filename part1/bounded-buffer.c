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
#include "bounded-buffer.h"

int main(int argc, char * argv[]) {

	int time_to_live;
	int num_prod_thread;
	int num_cons_thread;
	long i;
	pthread_t * prod_threads;
	pthread_t * cons_threads;

	// Assignment requires the correct amount of arguments.
	if (argc != 4 && argc != 5)
	{
		printf("Invalid number of arguments! You currently have %d\n", argc);
		exit(-1);
	}

	time_to_live = atoi(argv[1]);
	num_prod_thread = atoi(argv[2]);
	num_cons_thread = atoi(argv[3]);
	// Assignment requires that the buffer be greater than 1.
	if (argc == 5)
	{
		buffer_size = atoi(argv[4]);
		if (buffer_size < 1)
		{
			printf("Invalid buffer size. Must be > 1.\n");
			exit(-1);
		}
	}

	// Initalize semaphore for the buffer.
	semaphore_create(&sem_buffer, 1);
	// Initalize semaphore for printf.
	semaphore_create(&sem_printf, 1);

	semaphore_wait(&sem_printf);

	printf("Buffer Size               :  %d\n", buffer_size);
	printf("Time To Live (seconds)    :  %d\n", time_to_live);
	printf("Number of Producer threads:  %d\n", num_prod_thread);
	printf("Number of Consumer threads:  %d\n", num_cons_thread);
	printf("-------------------------------\n");

	printf("Initial Buffer:                          ");

	// Allocate space in production threads
	prod_threads = malloc(sizeof(pthread_t) * num_prod_thread);
	// Allocate space in consumer threads
	cons_threads = malloc(sizeof(pthread_t) * num_cons_thread);
	// Allocate space in buffer;
	buffer = malloc(sizeof(buffer_item) * buffer_size);

	// Fill buffer with -1 values to show non-produced/consumed values.
	for (i = 0; i < buffer_size; i++)
	{
		buffer[i] = -1;
	}

	// Print the buffer.
	print_buffer();
	semaphore_post(&sem_printf);

	// Seed the random number generator for use of random numbers later.
	srandom(time(NULL));

	for (i = 0; i < num_prod_thread; i++)
	{
		// Create pthreads for producers
		pthread_create(&prod_threads[i], NULL, producer, (void *)i);
	}

	for (i = 0; i < num_cons_thread; i++)
	{
		// Create pthreads for consumers
		pthread_create(&cons_threads[i], NULL, consumer, (void *)i);
	}
	
	sleep(time_to_live);

	// Set stop signal to 1, to stop pthreads.
	stop_signal = 1;

	if (is_debug == 1)
	{
		printf("Waiting for threads to finish...\n");
	}

	// Wait for pthreads to finish.
	for (i = 0; i < num_prod_thread; i++)
	{
		pthread_join(prod_threads[i], NULL);
	}
	for (i = 0; i < num_cons_thread; i++)
	{
		pthread_join(cons_threads[i], NULL);
	}

	// Print totals
	semaphore_wait(&sem_printf);
	printf("-----------------------\n");
	printf("Produced    |    %d    \n", total_prod);
	printf("Consumed    |    %d    \n", total_cons);
	printf("-----------------------\n");
	semaphore_post(&sem_printf);

	if (is_debug == 1)
	{
		printf("Destroying semaphores...\n");
	}

	// Remove semaphores from memory
	semaphore_destroy(&sem_buffer);
	semaphore_destroy(&sem_printf);

	for (i = 0; i < buffer_size; i++)
	{
		buffer[i] = 0;
	}

	free(buffer);
	buffer = NULL;

    return 0;
}

void *producer(void *threadid)
{
	long tid;
	tid = (long)threadid;
	int rand_num;

	// Continue until stop signal is given.
	while (stop_signal == 0)
	{
		usleep(random() % 2);

		// Try to get the semaphore for the buffer.
		if (semaphore_trywait(&sem_buffer) == 0)
		{
			// Do not overlap the consumer semaphore.
			if (prod_where + 1 != cons_where && !(prod_where == buffer_size - 1 && cons_where == 0))
			{
				// Add random value to buffer
				rand_num = random() % 10;
				buffer[prod_where] = rand_num;

				total_prod++;
				// Start the producer at the beginning, if at end.
				if (prod_where == buffer_size - 1)
				{
					prod_where = 0;
				}
				else
				{
					prod_where++;
				}

				// Print what this thread just did.
				semaphore_wait(&sem_printf);
				printf("Producer %4ld: Total %4d, Value %4d    ", tid, total_prod, rand_num);
				print_buffer();
				semaphore_post(&sem_printf);
			}

			// Allow other threads to access buffer.
			semaphore_post(&sem_buffer);
		}
	}

	// Exit thread gracefully.
	pthread_exit(NULL);
}

void *consumer(void *threadid)
{
	long tid;
	tid = (long)threadid;
	int removed_num;

	// Continue until stop signal is given.
	while (stop_signal == 0)
	{
		usleep(random() % 2);

		// Try to get the semaphore for the buffer.
		if (semaphore_trywait(&sem_buffer) == 0)
		{
			// Do not overlap the producer semaphore.
			if (cons_where != prod_where)
			{
				// Remove random value from buffer.
				removed_num = buffer[cons_where];
				buffer[cons_where] = -1;

				total_cons++;
				// Start the consumer at the beginning, if at end.
				if (cons_where == buffer_size - 1)
				{
					cons_where = 0;
				}
				else
				{
					cons_where++;
				}

				// Print what this thread just did.
				semaphore_wait(&sem_printf);
				printf("Consumer %4ld: Total %4d, Value %4d    ", tid, total_cons, removed_num);
				print_buffer();
				semaphore_post(&sem_printf);
			}

			// Allow other threads to access buffer.
			semaphore_post(&sem_buffer);
		}
	}

	// Exit thread gracefully.
	pthread_exit(NULL);
}

void print_buffer()
{
	int i;

	printf("[");
	for (i = 0; i < buffer_size; i++)
	{
		// Print each value in buffer.
		printf(" %2d", buffer[i]);
		// Show where producer and consumer are at.
		if (prod_where == i)
		{
			printf("^");
		}
		if (cons_where == i)
		{
			printf("v");
		}
	}
	printf("]\n");
}