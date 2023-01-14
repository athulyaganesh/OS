/* 
OS HW6 TASK 2 
AUTHOR: ATHULYA GANESH 
RW_LOCK_TEST-4c.c 
*/

// Homework Seven Sample Code
//
// This code is intended to serve as a working example of why one might even want
// different locking primitives AND an application you can use to test your
// implementation of a reader-writer lock.
//
// The best way to use this code is to read ALL the comments and line them up with what
// code you're seeing.  You might also wish to think about what would happen if the
// code were modified in certain ways.  The comments will invite you to do some
// specific experimental modifications, but you should not feel restricted to doing
// only those.
//
// So, what is this code even about?
//
// Well, this code is intended to run N threads.  One of those threads fills a large
// array with numbers.  The array starts out filled all with zeros, and the "array fill"
// thread iterates through it and fills each slot of the array with a value equal to the
// index of the slot.  So, when the "filler" thread is done, an array of 1000 slots will
// hold 0 in the first slot, 1 in the next, 2 in the next, all the way up to 999.
// The other N-1 threads are "readers".  In this case, all of these threads are
// constantly computing a "checksum" to see if the data structure is being filled
// correctly.  Basically, each of these threads will manually add up all the numbers in
// the array and see if that sum is consistent with what one would get evaluating a
// summation formula that should give the same result.  If at any point, a "reader"
// thread doesn't see what it expects, it complains IMMEDIATELY by printing an error
// message to standard output.
//
// Now if I were to run the "filler" first, and then serially run a "reader/checker"
// after, I'd expect no problems at all.  However, if I were to run the "filler" and
// some number of "reader/checkers" concurrently or in parallel, I'd expect the
// possibility of shenanigans and race conditions.  Only a proper use of synchronization
// primitives could really prevent this.
//
// Now, the code I've given you, by default, uses normal pthread_mutex operators to make
// the code safe.  If you've done the reading for the assignment, you know that for this
// particular application, if there are more readers than writers and if writing
// operations were relatively rare, this wouldn't be efficient (even if safe).
//
// Be sure to continue reading the comments inline with the code for more insight into
// what is going on and what the issues would be for  1) Using no synchronization;
// 2) using standard mutex; and 3) using reader-writer locks.
//
// Following are all of the include blocks.  Note that I've commented out the files
// I've asked you to write for the homework.  When you write your own primitives
// according to the specs of the assignment, you can #include them in the indicated
// places below (this is the cheezy hack way of doing it) or you can create a Makefile
// to separately compile, and link as is the "preferred" way.  Either is fine for this
// assignment.

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
// #include "RW_lock.h"
#include "RW_lock.c"

// I'm working with a "fill array" of 1000 slots.  Feel free to experiment with this
// number

#define ARRAY_SIZE 1000

// In this code, I'm using a standard mutex.  When you test your read-write lock,
// you'll want to uncomment the line after it.  BE SURE YOUR CODE WORKS WITH THE
// RW_lock_t array_lock declaration below.  YOU MUST FOLLOW THE API SPEC IN THE HOMEWORK
// OR WE WON'T BE ABLE TO GRADE YOUR ASSIGNMENT.

// pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
RW_lock_t array_lock;

// This is the global data that will be shared by all the threads.  big_array[]
// is the array that is being filled and beging (concurrently or in parallel)
// checked by the other threads.  Data_Insert_Count will contain the count of items
// already "read" into the big_array[].  As the filler thread runs, it will increment
// this count appropriately.  The reader threads will use this variable to also monitor
// how many values have been read and will use that number to compute its "checksum"

int big_array[ARRAY_SIZE];
int Data_Insert_Count = 0;

// The function *put_item_in_array() will become the code associated with the
// "fill the array" thread. Note that it iterates until the array have been given
// ARRAY_SIZE values (I.E. The array becomes full).
// In its default form, I have NO synch primitives in use.  You can imagine in
// that form it's not safe...

void *
put_item_in_array ()
{
  int index;
  int data;
  int sum;
  int sum_predicted;

  printf ("write_thread\n");
  while (Data_Insert_Count < ARRAY_SIZE)
    {
      // One one of the next two code lines needs to be uncommented.  One is for
      // locking with a standard mutex and the other is for locking with the
      // read-write lock you'll create

      //       pthread_mutex_lock(&mutex_lock);
      RW_write_lock (&array_lock);

      big_array[Data_Insert_Count] = Data_Insert_Count;
      Data_Insert_Count++;

      // One one of the next two code lines needs to be uncommented.  One is for
      // locking with a standard mutex and the other is for locking with the
      // read-write lock you'll create  Your choice here should correspond to the
      // choice you made earlier in this code.

      //       pthread_mutex_unlock(&mutex_lock);
      RW_write_unlock (&array_lock);
    }

  pthread_exit (0);
}

// The function *array_checksum() will CONSTANTLY (at least until the array is full)
// run through the array, summing its contents, and checking if that sum is consistent
// with what SHOULD be there as predicted by evaluating the proper summation formula
// that should, evaluated for the number of things loaded into the array, predict it.
// Note that when run as a parallel or concurrent thread.... this code is CONSTANTLY
// reading and checking.  Of course, it shouldn't be doing either if anyone is WRITING.
// It totally COULD be doing that so long as other active threads are ONLY reading.
// So... maybe using normal mutex is overkill? :)
//
// Maybe you might want to run this code with NO sync primitives, with standard mutex,
// and with your reader-writer locks.  It would be interesting to use the unix time
// command to see how long the code takes to complete under various situations and with
// various numbers of reader threads.  With this code, you'll find a simple shell
// script that you can use to run the program multiple times.  You could pipe the shell's
// output to a file to make records of the results of multiple experiments.  You could
// include time commands in my shell to capture records of run time....
//
// fun fun....
//

void *
array_checksum ()
{
  int index;
  int data;
  int sum;
  int sum_predicted;

  while (Data_Insert_Count < ARRAY_SIZE)
    {				// One one of the next two code lines needs to be uncommented.  One is for
      // locking with a standard mutex and the other is for locking with the
      // read-write lock you'll create

      //      pthread_mutex_lock(&mutex_lock);
      RW_read_lock (&array_lock);

      sum = 0;
      for (index = 1; index < ARRAY_SIZE; index++)
	sum += big_array[index];
      sum_predicted = ((Data_Insert_Count - 1) * (Data_Insert_Count)) / 2;
      if (sum != sum_predicted)
	printf
	  ("ERROR: INCONSISTENCY IN BIG ARRAY!  sum=%d   predicted sum =%d  items_inserted=%d\n",
	   sum, sum_predicted, Data_Insert_Count);

      // One one of the next two code lines needs to be uncommented.  One is for
      // locking with a standard mutex and the other is for locking with the
      // read-write lock you'll create  Your choice here should correspond to the
      // choice you made earlier in this code.

      //      pthread_mutex_unlock(&mutex_lock);
      RW_read_unlock (&array_lock);
    }

  pthread_exit (0);
}

int
main ()
{
  int c;			// I'll use this for generic counts
  pthread_t put_thread;		// The descriptor for the single "writer" thread
  pthread_t check_thread[100];	// Descriptors for up to 100 "reader" threads
  int check_thread_count = 20;	// A variable to hold how many reader threads
  // I want to create.  Keep in mind that the more
  // reader threads there are, the more contention
  // for the use of the single array resource.
  // The number of threads, AND your choice of
  // synchronization primitive may have profound
  // effects on how fast your code (as a whole
  // application) runs.  Also affecting how fast
  // things run would be the specific implementation
  // of your own reader-writer lock (when you
  // start using it).  Note that whenever dealing
  // with wibblywoblytimeywimey stuff, all the
  // details matter.

  // Init the RW lock
  // You'll need to uncomment this when using your reader-writer library code.
  // This call is in the API defined in the homework that is intended to init
  // the structure that contains all the book keeping variables associated with
  // your implementation of the reader-writer lock.

  RW_lock_init (&array_lock);

  // First, zero out the contents of the big_array.  I want to start with it holding
  // all zeros.

  for (c = 0; c < ARRAY_SIZE; c++)
    big_array[c] = 0;

  // Now, launch the threads
  // I'm launching exactly ONE filler thread and <check_thread_count> reader "checksum"
  // threads.  Note that when they are launched, they are ALL running in parallel and/or
  // concurrently and they are all doing their jobs "as quickly" as the kernel
  // scheduler will let them.  If you have your critical section protection correct,
  // NONE of the reader/checksum threads will ever print a complaint message.  Note
  // that it is entirely possible that unprotected threads don't mess up.  The point is
  // that they CAN mess up, and will eventually if you run the code long enough.

  pthread_create (&put_thread, NULL, &put_item_in_array, NULL);
  for (c = 0; c < check_thread_count; c++)
    pthread_create (&check_thread[c], NULL, &array_checksum, NULL);

  // This part is easy.  I'm just having main() request that all the threads I
  // launched rejoined.  Assuming you get your locking and unlocking right, all
  // threads should eventually terminate and be allowed to rejoin :)

  pthread_join (put_thread, NULL);
  for (c = 0; c < check_thread_count; c++)
    pthread_join (check_thread[c], NULL);
}
