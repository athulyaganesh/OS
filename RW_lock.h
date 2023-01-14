/* 
OS HW6 TASK 2 
AUTHOR: ATHULYA GANESH 
RW_lock.h HEADER FILE 
*/

#ifndef RW_LOCK_H_
#define RW_LOCK_H_

typedef int simple_spinlock_t;

typedef struct RW_lock_s
{				/* a field for every bookkeeping variable that is needed by your
				   Implementation */
  int num_readers;
  pthread_mutex_t read_lock;
  simple_spinlock_t write_lock;
} RW_lock_t;

void RW_lock_init (RW_lock_t * lock);
/* This routine should be called on a pointer to a struct variable of RW_lock_t
   to initialize it and ready it for use. */

void RW_read_lock (RW_lock_t * lock);
/* This routine should be called at the beginning of a READER critical section */

void RW_read_unlock (RW_lock_t * lock);
/* This routine should be called at the end of a READER critical section */

void RW_write_lock (RW_lock_t * lock);
/* This routine should be called at the beginning of a WRITER critical section */

void RW_write_unlock (RW_lock_t * lock);
/* This routine should be called at the end of a WRITER critical section */

#endif
