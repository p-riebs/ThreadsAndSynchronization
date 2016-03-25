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
#include "finicky-voter.h"

int main(int argc, char * argv[]) {

    int num_rep = 5;
    int num_dem = 5;
    int num_ind = 5;
    int i;
    pthread_t * rep_threads;
    pthread_t * dem_threads;
    pthread_t * ind_threads;

    int * thread_args = (int *)malloc(2 * sizeof(int));

    // Assignment requires the correct amount of arguments.
    if (argc > 6)
    {
        printf("Invalid number of arguments! You currently have %d\n", argc);
        exit(-1);
    }

    // Check if arguments are greater than 0
    for (i = 1; i < argc; i++)
    {
        if (atoi(argv[i]) < 1)
        {
            printf("Invalid argument. All arguments must be > 1.\n");
            exit(-1);
        }
    }

    // Assign arguments to variables.
    if (argc > 1)
    {
        num_vote_booth = atoi(argv[1]);
    }
    if (argc > 2)
    {
        num_rep = atoi(argv[2]);
    }
    if (argc > 3)
    {
        num_dem = atoi(argv[3]);
    }
    if (argc > 4)
    {
        num_ind = atoi(argv[4]);
    }

    printf("Number of Voting Booths   :   %d\n", num_vote_booth);
    printf("Number of Republican      :   %d\n", num_rep);
    printf("Number of Democrat        :   %d\n", num_dem);
    printf("Number of Independent     :   %d\n", num_ind);
    printf("-----------------------+-----------------------+--------------------------------\n");

    // Allocate space in republican threads
    rep_threads = malloc(sizeof(pthread_t) * num_rep);
    // Allocate space in democrat threads
    dem_threads = malloc(sizeof(pthread_t) * num_dem);
    // Allocate space in independent threads
    ind_threads = malloc(sizeof(pthread_t) * num_ind);
    // Allocate space in voting booth for how many user defined voting booths
    // thare are. Default 10.
    voting_booths = malloc(sizeof(vote_booth) * num_vote_booth);

    // All voting booths start empty, fill voting booths with -1;              
    for (i = 0; i < num_vote_booth; i++)
    {
        voting_booths[i] = -1;
    }

    // Seed the random number generator for use of random numbers later.
    srandom(time(NULL));

    semaphore_create(&sem_poll_station, 0);
    semaphore_create(&sem_queue, 1);
    semaphore_create(&sem_repub_dem_wait, 1);
    semaphore_create(&sem_printf, 1);
    semaphore_create(&mutex_voting_booth, 1);
    semaphore_create(&mutex_party_currently_voting, 1);
    semaphore_create(&mutex_second_line_count, 1);
    sem_count_create(&sem_booths, num_vote_booth);

    // Create threads for republicans
    for (i = 0; i < num_rep; i++)
    {
        thread_args[0] = i;
        thread_args[1] = 0;
        pthread_create(&rep_threads[i], NULL, voter, (void *)thread_args);
    }
    // Create threads for democrats
    for (i = 0; i < num_dem; i++)
    {
        thread_args[0] = i;
        thread_args[1] = 1;
        pthread_create(&dem_threads[i], NULL, voter, (void *)thread_args);
    }
    // Create threads for independents
    for (i = 0; i < num_ind; i++)
    {
        thread_args[0] = i;
        thread_args[1] = 2;
        pthread_create(&ind_threads[i], NULL, voter, (void *)thread_args);
    }

    sleep(2);

    semaphore_wait(&sem_printf);
    printf("-----------------------+-----------------------+--------------------------------\n");
    semaphore_post(&sem_printf);

    semaphore_post(&sem_poll_station);

    // Wait for voters to finish.
    for (i = 0; i < num_rep; i++)
    {
        pthread_join(rep_threads[i], NULL);
    }
    for (i = 0; i < num_dem; i++)
    {
        pthread_join(dem_threads[i], NULL);
    }
    for (i = 0; i < num_ind; i++)
    {
        pthread_join(ind_threads[i], NULL);
    }

    printf("-----------------------+-----------------------+--------------------------------\n");

    // Clean up.
    if (is_debug == 1)
    {
        printf("Destroying semaphores...\n");
    }

    // Remove all semaphores from memory.
    semaphore_destroy(&sem_poll_station);
    semaphore_destroy(&sem_queue);
    semaphore_destroy(&sem_repub_dem_wait);
    semaphore_destroy(&mutex_voting_booth);
    semaphore_destroy(&sem_printf);
    semaphore_destroy(&mutex_party_currently_voting);
    semaphore_destroy(&mutex_second_line_count);
    sem_count_destroy(&sem_booths);

    // Remove threads from memory.
    free(rep_threads);
    rep_threads = NULL;
    free(dem_threads);
    dem_threads = NULL;
    free(ind_threads);
    ind_threads = NULL;

    // Remove booths from memory.
    for (i = 0; i < num_vote_booth; i++)
    {
        voting_booths[i] = 0;
    }

    free(voting_booths);
    voting_booths = NULL;

    return 0;
}

void * voter(void * thread_args)
{
    int *args = (int *)thread_args;
    int tid = args[0];
    int party = args[1];
    int index_booth;
    semaphore_t my_sem;

    // Waiting for polling station.
    print_voting_status(party, tid, 0, 0);

    // All voters will wait to be triggered by main.
    semaphore_wait(&sem_poll_station);
    // Once the first voter is allowed to enter, immediately allow all other
    // voters to enter as well.
    semaphore_post(&sem_poll_station);

    // Enter the polling station
    print_voting_status(party, tid, 1, 0);

    // Take time to sign in.
    usleep(random() % 50000);

    // Keeps the line of who is ready to wait/vote at booths.
    semaphore_create(&my_sem, 0);

    semaphore_wait(&sem_queue);
    // Add this voter thread's semaphore to the queue
    if (queue_head == NULL)
    {
        enqueue(&my_sem);

        semaphore_post(&sem_queue);
    }
    else
    {
        enqueue(&my_sem);

        semaphore_post(&sem_queue);
        // Wait for signal if there is another thread currently in the critical
        // section.
        semaphore_wait(&my_sem);
    }

    if (is_debug)
    {
        semaphore_wait(&sem_printf);
        printf("Party %d, ID %d - Can wait/vote\n", party, tid);
        semaphore_post(&sem_printf);
    }

    // Check for each individual voter are allowed to go wait/vote at the
    // booths because of the party they represent.
    semaphore_wait(&mutex_party_currently_voting);
    // The other major party are voting/waiting at booths.
    if (party_currently_voting != party && party != 2)
    {
        if (is_debug)
        {
            semaphore_wait(&sem_printf);
            printf("Party %d, ID %d - Party_currently_voting %d\n", party, tid, party_currently_voting);
            semaphore_post(&sem_printf);
        }
        // Immediately give up party currently voting mutex to avoid deadlock.
        semaphore_post(&mutex_party_currently_voting);
        // Need to wait for other party to be done voting.
        semaphore_wait(&sem_repub_dem_wait);
        // If this voter can go to wait/vote at voting booths, change the 
        // party that is currently voting to the party represented by this 
        // voter.
        semaphore_wait(&mutex_party_currently_voting);
        party_currently_voting = party;
        if (is_debug)
        {
            semaphore_wait(&sem_printf);
            printf("Party %d, ID %d - Changed to Party %d\n", party, tid, party);
            semaphore_post(&sem_printf);
        }
        semaphore_post(&mutex_party_currently_voting);
    }
    else
    {
        semaphore_post(&mutex_party_currently_voting);
    }

    // Count of how many major party voters that are waiting/voting at a 
    // voting booth.
    if (party != 2)
    {
        semaphore_wait(&mutex_second_line_count);
        second_line_count++;
        semaphore_post(&mutex_second_line_count);
    }

    semaphore_wait(&sem_queue);

    // Remove self from queue.
    dequeue();
    // If there is another thread in the queue, signal it.
    if (queue_head != NULL)
    {
        if (is_debug)
        {
            semaphore_wait(&sem_printf);
            printf("Party %d, ID %d - Going to post the next semaphore in the queue.\n", party, tid);
            semaphore_post(&sem_printf);
        }
        semaphore_post(queue_head->my_sem);
    }

    semaphore_post(&sem_queue);

    // Waiting on a voting booth to open up.
    print_voting_status(party, tid, 2, 0);

    // Counting semaphore to make sure voting booths aren't all full.
    if (is_debug)
    {
        semaphore_wait(&sem_printf);
        printf("Party %d, ID %d - Waiting on booth to open.\n", party, tid);
        semaphore_post(&sem_printf);
    }
    sem_count_wait(&sem_booths);
    if (is_debug)
    {
        semaphore_wait(&sem_printf);
        printf("Party %d, ID %d - Booth is open.\n", party, tid);
        semaphore_post(&sem_printf);
    }

    // Find empty booth, print status.
    semaphore_wait(&mutex_voting_booth);
    index_booth = find_empty_booth(party);
    print_voting_status(party, tid, 3, index_booth);
    semaphore_post(&mutex_voting_booth);

    // Thoughtfully vote.
    usleep(random() % 100000);

    // Done voting, remove self from voting booth, and total amount of people
    // voting in a booth.
    semaphore_wait(&mutex_voting_booth);
    voting_booths[index_booth] = -1;
    print_voting_status(party, tid, 4, 0);
    sem_count_post(&sem_booths);
    semaphore_post(&mutex_voting_booth);

    // Remove self from how many of my party is waiting/voting at voting booths.
    if (party != 2)
    {
        semaphore_wait(&mutex_second_line_count);
        second_line_count--;
        // If there are no more of my party at the voting booths, allow other
        // parties to enter as well.
        if (second_line_count == 0)
        {
            semaphore_wait(&mutex_party_currently_voting);
            party_currently_voting = -1;
            semaphore_post(&mutex_party_currently_voting);
            semaphore_post(&sem_repub_dem_wait);
        }
        semaphore_post(&mutex_second_line_count);
    }

    // Exit thread gracefully.
    pthread_exit(NULL);
}

void print_voting_status(int party, int tid, int status, int booth_index)
{
    semaphore_wait(&sem_printf);
    // check party to print
    if (party == 0)
    {
        printf("Republican  ");
    }
    else if (party == 1)
    {
        printf("Democrat    ");
    }
    else if (party == 2)
    {
        printf("Independent ");
    }
    printf("%3d", tid);
    if (status == 3)
    {
        printf(" in %3d", booth_index);
    }
    else
    {
        printf("       ");
    }
    // print the booths
    print_booth_status();
    // print what status the user is.
    if (status == 0)
    {
        printf("Waiting for polling station to open...\n");
    }
    else if (status == 1)
    {
        printf("Entering the polling station\n");
    }
    else if (status == 2)
    {
        printf("Waiting on a voting booth\n");
    }
    else if (status == 3)
    {
        printf("Voting!\n");
    }
    else if (status == 4)
    {
        printf("Leaving the polling station\n");
    }
    semaphore_post(&sem_printf);
}

void print_booth_status()
{
    int i;
    printf(" |-> ");
    for (i = 0; i < num_vote_booth; i++)
    {
        printf("[");
        // -1: Empty booth
        if (voting_booths[i] == -1)
        {
            printf(".");
        }
        // 0: Republican
        else if (voting_booths[i] == 0)
        {
            printf("R");
        }
        // 1: Democrat
        else if (voting_booths[i] == 1)
        {
            printf("D");
        }
        // 2: Independent
        else if (voting_booths[i] == 2)
        {
            printf("I");
        }
        printf("]");
    }
    printf(" <-| ");
}

int find_empty_booth(int party)
{
    int index_booth = -1;
    int i;

    for (i = 0; i < num_vote_booth; i++)
    {
        if (voting_booths[i] == -1)
        {
            voting_booths[i] = party;
            index_booth = i;
            // Find empty booth break out of loop
            break;
        }
    }

    return index_booth;
}

void sem_count_create(sem_count_t * sem_count, int count)
{
    semaphore_create(&sem_count->mutex, 1);
    semaphore_create(&sem_count->waiting, 0);
    sem_count->count = count;

    return;
}

void sem_count_wait(sem_count_t * sem_count)
{
    semaphore_wait(&sem_count->mutex);
    sem_count->count--;
    if (sem_count->count < 0)
    {
        semaphore_post(&sem_count->mutex);
        semaphore_wait(&sem_count->waiting);
    }
    semaphore_post(&sem_count->mutex);

    return;
}

void sem_count_post(sem_count_t * sem_count)
{
    semaphore_wait(&sem_count->mutex);
    sem_count->count++;
    if (sem_count->count <= 0)
    {
        semaphore_post(&sem_count->waiting);
    }
    else
    {
        semaphore_post(&sem_count->mutex);
    }

    return;
}

void sem_count_destroy(sem_count_t * sem_count)
{
    semaphore_destroy(&sem_count->mutex);
    semaphore_destroy(&sem_count->waiting);
    sem_count->count = 0;

    sem_count = NULL;
}

void enqueue(semaphore_t * my_sem)
{
    queue_sem_voter * temp = malloc(sizeof(queue_sem_voter));
    temp->my_sem = (semaphore_t *)malloc(sizeof(semaphore_t));
    temp->my_sem = my_sem;
    temp->ptr = NULL;

    if (queue_head == NULL && queue_rear == NULL)
    {
        queue_head = temp;
        queue_rear = temp;
        return;
    }

    queue_rear->ptr = temp;
    queue_rear = temp;
}

void dequeue()
{
    queue_sem_voter * temp = queue_head;
    if (queue_head == NULL)
    {
        printf("Queue is empty, can't dequeue!\n");
        return;
    }

    if (queue_head == queue_rear)
    {
        queue_head = NULL;
        queue_rear = NULL;
    }
    else
    {
        queue_head = queue_head->ptr;
    }
    // Remove semaphore from memory
    semaphore_destroy(temp->my_sem);
    temp->my_sem = NULL;
    // Remove semaphore voter from memory.
    free(temp);
}