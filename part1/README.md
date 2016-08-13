# Bounded Buffer

## Author(s):

Parker Riebs


## Date:

3/8/2016


## Description:

Creates a bounded buffer for a user defined amount of buffer space,
producers, and consumers. Fills the buffer with random numbers between
0-9. Then consumes the random number. This is repeated until the
"Time-to-live" variable is up.

## How to build the software

run "make"

## How to use the software

Run with the arguments:
1: The time for the process to live.
2: The number of producer threads.
3: the number of consumer threds.
4: (optional) the buffer size (default: 10)
 
## How the software was tested

I ran the software through a few tests. Since testing multi-threaded 
processers is difficult since there many different ways a threads can
be intertwined. But, there should be no problems since producers and
consumers will take their turn using the buffer using the semaphore
buffer to avoid conflicting critical sections.

## Known bugs and problem areas

Producers and consumers use busy waiting when using trywait and sleep
to avoid deadlock.

