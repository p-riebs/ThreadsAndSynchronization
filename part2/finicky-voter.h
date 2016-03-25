/*
 * Parker Riebs
 * 3/24/2016
 *
 * Creates a simulation of a voting booth. Democrats and republicans are not
 * allowed to wait in the same line for voting booths, but independents can
 * vote with anyone. Voting booths are displayed of which party the voter
 * represents, and the status of that voter shown as well. Fairness of the line
 * to keep a queue is also implemented.
 *
 * CS 441/541: Finicky Voter (Project 3 Part 2)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "semaphore_support.h"

/*****************************
 * Defines
 *****************************/

/*****************************
 * Structures
 *****************************/
// Counting semaphore struct
struct sem_count_t {
    semaphore_t mutex;
    semaphore_t waiting;
    int count;
};
typedef struct sem_count_t sem_count_t;

// Queue of semaphore data struct.
struct queue_sem_voter {
    semaphore_t * my_sem;
    struct queue_sem_voter * ptr;
};
typedef struct queue_sem_voter queue_sem_voter;

typedef int vote_booth;

/*****************************
 * Global Variables
 *****************************/
int is_debug = 0;
// Shows that the polling station is open are not.
// Starts closed and is initially opened after 2 seconds.
semaphore_t sem_poll_station;

// Keeps a single democrat/republican thread to stop sem_all_parties
// from allowing all voters to go through.
semaphore_t sem_repub_dem_wait;

// Hold status of what party is voting in what booth.
vote_booth * voting_booths;
int num_vote_booth = 10;
// Allow access to view/manipulate which parties are in voting booths.
semaphore_t mutex_voting_booth;

// Counting semaphore to make sure only the user defined amount of voting
// booths are being used at the same time and no more.
sem_count_t sem_booths;

// Make sure printf commands are not conflicting with other threads.
semaphore_t sem_printf;

// Shows which party is currently allowed to move from polling station
// to voting/waiting at voting booth.
semaphore_t mutex_party_currently_voting;
// -1: No party is currently holding the booths
//  0: Republicans are holding the booths
//  1: Democrats are holding the booths
int party_currently_voting = -1;

// The amount of voters waiting/voting at the voting booths.
semaphore_t mutex_second_line_count;
int second_line_count;

// Allows only one thread at a time to use the queue.
semaphore_t sem_queue;
// Keeps the line of who is ready to wait/vote at booths.
queue_sem_voter * queue_head = NULL;
queue_sem_voter * queue_rear = NULL;

/*****************************
 * Function Declarations
 *****************************/
// Main function of voter to wait in line and vote at the voting booths, then
// leave.
void * voter(void * thread_args);

// Prints the voting booth status.
void print_booth_status();

// Finds an empty booth in the voting booth array, returns what index of the 
// voting booths the thread got.
int find_empty_booth(int party);

// Prints the full status of the voter.
void print_voting_status(int party, int tid, int status, int booth_index);

// Implement counting semaphore with binary semaphores and counter.
void sem_count_create(sem_count_t * sem_count, int count);
void sem_count_wait(sem_count_t * sem_count);
void sem_count_post(sem_count_t * sem_count);
// Remove counting semaphore
void sem_count_destroy(sem_count_t * sem_count);

// Implement a queue with the linked list method.
void enqueue(semaphore_t * my_sem);
void dequeue();