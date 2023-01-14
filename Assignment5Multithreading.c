//CS 4029 OPERATING SYSTEMS
//HOMEWORK 5 PTHREADS
// MULTITHREADING 
// AUTHOR: ATHULYA GANESH 


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>  

double c[20]; // global array to store the initial randomly generated array (size = 20)
double f[20]; // global array to store the final sorted array (also size = 20)
  
// struct to pass in parameters into the thread, and to return values as well
typedef struct arg  // threads 1 and 2 use this
          { int start_index;  //input parameter
	    double sublist[10];   // space for return value
          }  arg_type;

typedef struct arg1 // thread 3 uses this
	{
		double sublist1[10]; // input parameter
		double sublist2[10]; //input parameter
	} arg_type1; 

typedef arg_type *arg_ptr;
typedef arg_type1 *arg_three; 

int comparator (const void *a, const void * b) // comparator function is used with the inbuilt qsort() function in C. 
{// compares two values in an array and returns values depending on the result of the comparison. 
	if(*(double*)a > *(double*)b) 
	{
		return 1;
	}
	else if (*(double*)a < *(double*)b)
	{
		return -1;
	}
	else
	{
		return 0; 
	}
	
}

void *sort(void *input) // this function is used to sort the 2 sublists (for threads 1 and 2)
{
	//variables local to the thread. 
	double sublist[10]; // to store the sorted sublist and return the value via same struct
	int start_index; 
	int i;  

	start_index = ((arg_ptr)input)->start_index; // copying parameter from the parameter block into local memory
	
	for(i = 0; i <=9; i++)
	{
		sublist[i] = c[start_index + i]; // initialising the array with the unsorted values from the original randomized array. 
	}

	qsort(sublist, 10, sizeof(double), comparator); //qsort() sorts the list


	for(i = 0; i <=9; i++)
	{
	((arg_ptr)input)->sublist[i] = sublist[i]; // returns the sorted array 
	}

	pthread_exit(0);
}

void *merge(void *input) // merges the two sorted sublists, and puts the sorted list into the global array "f". (used by thread 3)
{

int i = 0, j = 0, k = 0; //local variables
double list1[10], list2[10]; 

for(i=0; i<=9; i++)
{
list1[i] = ((arg_three)input)->sublist1[i]; // storing parameters from paraemter block into local variables to use. 
list2[i] = ((arg_three)input)->sublist2[i]; 
}
i=0;
while(i<=9 && j<=9)
{
	if(list1[i] < list2[j])
	{
		f[k] = list1[i];
		k++;
		i++;
	}
	else if(list1[i] == list2[j])
	{
		f[k] = list1[i]; 
		k++;
		i++;
		f[k] = list2[j];
		k++;
		j++; 
	}
	else
	{
		f[k] = list2[j];
		k++;
		j++;  
	}
}

while(i<=9)
{
	f[k] = list1[i];
	k++;
	i++; 
}	
while(j<=9)
{
	f[k++] = list2[j++];
	k++;
	j++;  
}

	pthread_exit(0); //thread exits. 
}



int main()
{
 
    int i; //counter variable used multiple times through the code. 

    int rt1, rt2, rt3; // return values from the routine that laundes threads
 
    pthread_t t1, t2, t3; // hold bookkeeping values for the three threads
    
    arg_type args_1, args_2; // creating structs to hold input parameters for the three threads. 
 
    arg_type1 args_3; 

    time_t t;

    srand((unsigned)time(&t));  
      
    for (i=0; i <= 19; i++) // putting random data into the global input array c[] 
	{
        c[i] = (double)((rand()%10000)/10.0);
	
	}
	printf("Randomly generated list: \n");
	for(i=0; i<=19; i++)
	{
		printf("%f\n", c[i]); //prints the randomly generated list to the console. 
	}
	printf("\n\n"); 

	     args_1.start_index = 0; // to make sure that each of our sorting threads work on different halves of the array. (Thread 1 works on elements 0-9 and thread 2 works on 10-19)
	     args_2.start_index = 10;
		// create the two threads 
           if((rt1=pthread_create( &t1, NULL, sort, (void *)&args_1)))                  
              printf("Thread creation failed: %d\n", rt1);
               
           if((rt2=pthread_create( &t2, NULL, sort, (void *)&args_2)))
              printf("Thread creation failed: %d\n", rt2);


          pthread_join(t1, NULL);
          pthread_join(t2, NULL);
// wait for threads to finish. 

	//taking the return values from the two threads and inputs this data into the third struct as input parameters. 
	for(i=0; i <= 9; i++)
	{
		args_3.sublist1[i] = args_1.sublist[i];
		args_3.sublist2[i] = args_2.sublist[i]; 
	}
	// creates the final thread that is responsible for merging. 
	 if((rt3=pthread_create(&t3, NULL, merge, (void *)&args_3)))
	 {
		printf("Thread creation failed: %d\n", rt3); 
	 }

	// waiting for thread to finish. 
	pthread_join(t3, NULL);
	
	// main thread prints out the final sorted list to the console. 
	printf("Sorted list: \n"); 
	for(i=0; i <= 19; i++)
	{
		printf("%f\n", f[i]); 
	}
	

	printf("\n"); 
    return 0;
 
}
