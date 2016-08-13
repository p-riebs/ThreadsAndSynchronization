# Finicky Voter

## Author(s):

Parker Riebs


## Date:

3/24/2016


## Description:

Creates a simulation of a voting booth. Democrats and republicans are not
allowed to wait in the same line for voting booths, but independents can
vote with anyone. Voting booths are displayed of which party the voter
represents, and the status of that voter shown as well. Fairness of the line
to keep a queue is also implemented.

## How to build the software

run "make"

## How to use the software

Run with the arguments:
1: (optional) The number of booths the voters are allowed to vote in. (default: 10)
2: (optional) The number of Republicans voters. (default: 5)
3: (optional) the number of Democrat voters. (default: 5)
4: (optional) the number of Independent voters (default: 5)
 
## How the software was tested

I ran many tests of the software with different voter threads, but since 
multi-threaded software is difficult to debug, I will explain my thought
process. I used semaphores with any variable that could be used globally
across threads. These semaphores are released as soon as I'm down with the
critical section I'm protecting. Semaphores are rarely interweaving, but
if they are carefull consideration was give to those cases.

See the PDF "SpecialSectionDocumentation.pdf"

## Known bugs and problem areas

None as far as I know.
