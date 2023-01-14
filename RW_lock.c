/* 
OS HW6 TASK 2 
AUTHOR: ATHULYA GANESH 
RW_lock.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "RW_lock.h"

// simple_spinlock_t will be the type we define for synchronization flags.  In this
// implementation, we'll just go with a simple int. Every "lock" we would need would
// be declared of type "simple_spinlock_t" and would be declared as global so that
// ALL threads could see them as needed.

// Now, we need to implement functions to initialize, lock, and unlock spinlock type
// variables (flags).  This is going to be kind of subtle because any checks and/or
// modifications of the flags MUST BE ATOMIC.  If they are NOT atomic, it is possible
// that the reads and/or writes to the same flag from different threads could have
// their assembly level instructions interleave, this will create race conditions.  It
// doesn't seem all that wise to build a tool to prevent race conditions that itself
// has race conditions ;)  Let's look at them one-by-one.

// Function: simple_spin_init(simple_spinlock_t *lock)
// Purpose:  This function should be used to initialize the spinlock variable to its
//           initial bunsetbstate prior to use. This is generally run before any
//           threads are launched.  Using a lock variable that has not been
//           initialized is bad, as the contents of memory are not necessarily zeroed
//           out as a consequence of allocation.
//
//           Note that there's a little weirdness here with the __asm__ operator.
//           this is a compile time memory fence, which we will discuss in some
//           detail later.  The short version of this is that optimizing compliers
//           can (and do) shuffle the order of assembly instructions in a way that
//           preserves the ''final effect' but is somehow more time efficient when
//           the compiled code runs. In a single-processor system, this is a fairly
//           harmless optimization.  In a multi-processing / multi-threaded system
//           we often must guarantee that operations on memory happen in exactly the order
//           we expect them to so that execution of memory accesses is not put into an order
//           that ups the chance of a race condition.  Compile time memory fences force a
//           desired ordering of the assembly language instructions
//           that the high level code compiles into.  The specific syntax to do this varies
//           from compiler to compiler and os to os.  What I''m showing you here is
//           good for linux and gcc and also MacOS X and clang (the current default C
//           in Mac OSX).
//


int
simple_spin_init (simple_spinlock_t * lock)
{				// The next line sets the compile time memory fence
  __asm__ __volatile__ ("":::"memory");
  // here we set the flag to zero. Apparently ''unlocked' is zero o'
  // Exciting, right?
  *lock = 0;
  return 0;
}


// Function: simple_spin_lock(simple_spinlock_t *lock)
// Purpose:  This function implements the conceptual lock() routine using spinlock
//           methods.  You should note that the very nature of this function is
//           an endless loop that breaks out only under a specific flag condition.
//           This is in essence exactly what a lock() should do'& ''wait', forever if
//           necessary, until the flag it''s waiting on is available.The interesting
//           thing here is the __sync_bool_compare_and_swap() thing.
//          This is a complier built-in that provides a very specific ATOMIC
//           functionality in an architecture specific way.  Remember, we said that
//           the checking and changing of a flag must be entirely atomic.  The syntax
//           given here is joint compiler / hardware support for that.
//
//           The prototype is:
//
//           int __sync_bool_compare_and_swap(type *ptr, type oldvalue, type newvalue)
//
//           what it does is:
//
//           If the current value contained in *ptr is equal to oldvalue, then write
//           newvalue into *ptr.  If the ''swap' happened (*ptr was actually equal to
//           oldvalue and *ptr got replaced with new value), then the function returns
//           a ''true', otherwise it returns a ''false'.  The KEY ITEM HERE is that ALL
//           of this is atomic '' while the comparing and swapping is going on, no
//           other thread can interfer with *ptr.  Remember, we needed this'& and
//           without compiler and hardware support, we''d be hard pressed to get it.
//
//        So, what''s going on in the code.  Well'& look at the inline comments :)



int
simple_spin_lock (simple_spinlock_t * lock)
{
  int i;

  // First, we put ourselves into an infinite loop.  The lock routine should NEVER
  // return until it actually sees that *lock is'& unlocked (in this case set to zero)
  // If you''re thinking this is dangerous in that there''s no promise any other thread
  // will ever unlock the flag, you''re correct. We''ll talk about techniques that help
  // keep track of what gets unlocked when, but for now we''ll just be careful.

  while (1)
    {				// Now we''re inside the infinite loop.  First, check the lock flag 5000
      // times using the atomic check and set operator we talked about before.
      // If it ever succeeds, simple_spin_lock() will succeed after atomically
      // locking the lock.

      for (i = 0; i < 5000; i++)
	if (__sync_bool_compare_and_swap (lock, 0, 1))
	  return 0;

      // Ok, so we did our 5000 checks and never got past the lock. Rather than
      // banging on the lock incessantly and never giving up the CPU, we''ll call
      // sched_yield() so the thread running simple_spin_lock() can jump off the
      // CPU for a bit.  NOTE THIS IS NOT THE SAME AS ENTERING A WAIT STATE. If
      // a thread is in a wait, it is on the wait queue of the kernel and is not
      // eligible to be scheduled onto a CPU until some event happens and the kernel
      // puts it back in the ''ready' queue.  Assuming the scheduling is fair, every
      // process will eventually get a shot and this thread will eventually return
      // to the CPU where it can bang on the lock 5000 times again.
      // Basically this guy bangs the door 5000 times, then gets to the back of the
      // CPU line and waits another turn to do the same.  Here''s where we get the name
      // ''spinlock'.  The wait() actively spins its wheels actively checking the lock
      // until it can get it.

      sched_yield ();
    }
}



// Function: simple_spin_unlock(simple_spinlock_t *lock)
// Purpose:  This function implements the conceptual unlock() routine using spinlock
//           methods.  This one just clears the flag and is to be run at the END of a
//           critical section.  The code for this implementation is IDENTICAL to the
//           code for initializing the flag.
//

int
simple_spin_unlock (simple_spinlock_t * lock)
{
  __asm__ __volatile__ ("":::"memory");
  *lock = 0;
  return 0;
}









void
RW_lock_init (RW_lock_t * lock)
{
  /* This routine should be called on a pointer to a struct variable of RW_lock_t
     to initialize it and ready it for use. */
  lock->num_readers = 0;	//initializing number of readers to 0
  pthread_mutex_init (&(lock->read_lock), NULL);	//initializing the read lock, with default mutex attributes 
  simple_spin_init (&(lock->write_lock));	// initializing the spin lock for the write 
}

void
RW_read_lock (RW_lock_t * lock)
{
  /* This routine should be called at the beginning of a READER critical section */

  //to preven race condition for the threads that call this function 
  pthread_mutex_lock (&(lock->read_lock));

  // increment number of readers. 
  lock->num_readers += 1;

  // if the first reader, then LOCK the mutex! 
  if (lock->num_readers == 1)
    {
      printf ("WRITE LOCK. UNABLE TO WRITE. \n");
      simple_spin_lock (&(lock->write_lock));
    }
// release the read mutex for this function 
  pthread_mutex_unlock (&(lock->read_lock));
}

void
RW_read_unlock (RW_lock_t * lock)
{
  /* This routine should be called at the end of a READER critical section */

  /// prevents race coditions for threads calling the function 
  pthread_mutex_lock (&(lock->read_lock));

  // decrement the number of readers
  lock->num_readers -= 1;


  // if this is the last mutex, u can unlock the write mutex! 
  if (lock->num_readers == 0)
    {
      printf ("WRITE UNLOCK. ABLE TO WRITE NOW.\n");
      simple_spin_unlock (&(lock->write_lock));
    }
// release read mutext for this function 
  pthread_mutex_unlock (&(lock->read_lock));
  usleep(1); 
}

void
RW_write_lock (RW_lock_t * lock)
{
  /* This routine should be called at the beginning of a WRITER critical section */
  simple_spin_lock (&(lock->write_lock));
}

void
RW_write_unlock (RW_lock_t * lock)
{
  /* This routine should be called at the end of a WRITER critical section */
  simple_spin_unlock (&(lock->write_lock));
}
