//OPERATING SYSTEMS HOMEWORK 7
// AUTHOR: ATHULYA GANESH 

// STANDARD ARBITER SOLUTION 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>


// Change the constant below to change the number of philosophers
// coming to lunch...

#define PHILOSOPHER_COUNT 10

// Change the constant below to change how many "meals" are on the table.  This is 
// the number of times that any philosopher can eat.  Once all the meals are gone, then
// the philosophers are sent home (the threads terminate).  Note it MAY be the case
// that the deadlock prevention scheme is NOT fair.  It may be possible that 
// not all philosophers get to eat the same amount of meals....

#define AVAILABLE_MEALS 1000000


// Each philosopher is represented by one thread.  Each thread independenly
// runs the same "think/start eating/finish eating" program.

pthread_t philosopher[PHILOSOPHER_COUNT];

// Each chopstick gets one mutex.  If there are N philosophers, there are
// N chopsticks.  That's the whole problem.  There's not enough chopsticks 
// for all of them to be eating at the same time.  If they all cooperate, 
// everyone can eat.  If they don't... or don't know how.... well.... 
// philosophers are going to starve.

pthread_mutex_t chopstick[PHILOSOPHER_COUNT];

// The next item is a global variable that keeps track of how many meals 
// have been eaten.  WHen it goes to zero, all the food is gone (so sad)
// and the threads exit.  Because this is a global variable that is accessed
// by multiple threads, I will also provide a mutex for it so I don't get 
// a race condition.  THIS IS NOT PART OF ANY DEADLOCK PREVENTION SCHEME.

//ATHULYAS COMMENT: Global mutext waiter so that each philosopher asks permission to pick a fork 
pthread_mutex_t waiter; 

int MEALS_ON_TABLE = AVAILABLE_MEALS;
pthread_mutex_t MEAL_COUNT_LOCK;

void *philosopher_program(int philosopher_number)
   {  // In this version of the "philosopher program", the philosopher
      // will think and eat until all the meals are gone
      
      int how_many_times_did_I_eat = 0;
      
      while (MEALS_ON_TABLE > 0)
         { // Philosophers always think before they eat.  They need to 
           // build up a bit of hunger....
         
           printf ("Philosopher %d is thinking\n", philosopher_number);
           usleep(1);  
           
           // That was a lot of thinking.... now hungry... this 
           // philosopher (who knows his own number) grabs the chopsticks
           // to her/his right and left.  The chopstick to the left of 
           // philosopher N is chopstick N.  The chopstick to the right
           // of philosopher N is chopstick N+1
           
           printf ("Philosopher %d wants chopsticks\n",philosopher_number);
           
           // ATHULYAS COMMENTS: ONLY ONE PHILOSOPHER GETS TO PICK UP BOTH FORKS/CHOPSTICKS 
           // AT ONE TIME 
           // EVERY OTHER PHILOSOPHER WAITS 
           
           pthread_mutex_lock(&waiter);
           
           

           pthread_mutex_lock(&chopstick[philosopher_number]);
           pthread_mutex_lock(&chopstick[(philosopher_number+1)%PHILOSOPHER_COUNT]);
           
           
           // ATHULYAS COMMENTS: UNLOCK WAITER ONCE PHILOSOPHER HAS PICKED BOTH UP
           pthread_mutex_unlock(&waiter);
           
           
           // Hurray, if I got this far I'm eating
           printf ("Philosopher %d is ready to eat\n",philosopher_number);

           usleep(2); // I spend twice as much time eating as thinking...
                      // typical....
           
           // I'm done eating.  Now put the chopsticks back on the table

           if (MEALS_ON_TABLE > 0)
              { how_many_times_did_I_eat++;
                pthread_mutex_lock(&MEAL_COUNT_LOCK);
                MEALS_ON_TABLE--;
                printf ("Philosopher %d finished eating, %d meals remaining\n",philosopher_number, MEALS_ON_TABLE);
                pthread_mutex_unlock(&MEAL_COUNT_LOCK);

              }
 
           else
              { printf("Philosopher %d didn't eat because no food was left\n", philosopher_number);
              }
              // PUT THEM DOWN IN ANY ORDER 
           pthread_mutex_unlock(&chopstick[philosopher_number]);
           pthread_mutex_unlock(&chopstick[(philosopher_number+1)%PHILOSOPHER_COUNT]);
           printf("Philosopher %d has placed chopsticks on the table\n", philosopher_number);
         } 

      printf("PHILOSOPHER %d GOT TO EAT %d TIMES\n", philosopher_number, how_many_times_did_I_eat);
      return(NULL);
     }

int main()
  {  int i;

   // Initialize the MEALS_ON_TABLE variable locking mutex
   pthread_mutex_init(&MEAL_COUNT_LOCK, NULL);
   
   //ATHULYAS COMMENTS: INITIALIZE WAITER MUTEX 
   
   pthread_mutex_init(&waiter, NULL); 
   
   // Initialize all the chopsticks as "unlocked" or "not held"
   for(i=0;i<PHILOSOPHER_COUNT;i++)
      pthread_mutex_init(&chopstick[i],NULL);

   // Create the philosopher threads  
   for(i=0;i<PHILOSOPHER_COUNT;i++)
      pthread_create(&philosopher[i],NULL,(void *)philosopher_program,(void *)i);

   // collect the philosophers when they are done....  if the philosopher threads 
   // never terminate, then these joins never happen.
   for(i=0;i<PHILOSOPHER_COUNT;i++)
      pthread_join(philosopher[i],NULL);

   // Once all the threads terminate, destroy all the chopsticks
   for(i=0;i<PHILOSOPHER_COUNT;i++)
      pthread_mutex_destroy(&chopstick[i]);

   return 0;
   }
