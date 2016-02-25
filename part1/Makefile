#
# Samantha Foley
# CS 441/541: Project 3 Part 1
#
#####################################################################
#
# Type "make" to compile your code
# 
# Type "make clean" to remove the executable (and any object files)
#
#####################################################################

CC=gcc
CFLAGS=-Wall -g -O0 -I../lib
LDFLAGS=-pthread

PROGS=bounded-buffer
#
# List all of the binary programs you want to build here
# Separate each program with a single space
#
all: $(PROGS)

#
# Bounded Buffer
#
bounded-buffer: bounded-buffer.c bounded-buffer.h ../lib/semaphore_support.o
	$(CC) -o bounded-buffer bounded-buffer.c ../lib/semaphore_support.o $(CFLAGS) $(LDFLAGS)

#
# Cleanup the files that we have created
#
clean:
	$(RM) $(PROGS) *.o
	$(RM) -rf *.dSYM
	cd ../lib && make clean
