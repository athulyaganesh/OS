//OS 4029
// HOMEWORK 4
// AUTHOR: ATHULYA GANESH 
// HW3_TASK6.c 

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h> 


//HELPER FUNCTIONS 
void staticSignal(int signum)
{
    static int sigCount = 0; // keeps track of whether this is the first or the second call of the staticSignal function.
    // if it is the first call, then the SIGINT is ignored, else it is honored, as the question asks. 
    if(sigCount == 0)
    {
       
        printf("PID %d received a SIGINT that is being ignored. \n", getpid()); 
         
        sigCount++; 
    }
    else if(sigCount == 1)
    {
        printf("PID %d received a SIGINT that is being honored. \n", getpid());
        exit(0); 
    }
    else
    {
        printf("This should never execute"); 
    }
}

//HELPER FUNCTIONS 
void countdown() // countsdown for 5 seconds. 
{
    for(int i = 0; i < 5; i++)
    {
        sleep(1); 
        printf("\nCountdown %d", 5 - i,"\n"); 
    }
}

int main(int argc, char **argv, char **envp)
{
    //subtask 1 --> Create 5 children that go into infinite loops
    pid_t returned_pid; 
    int kidspids[5]; // created an array to store all the childrens PIDS 
    for(int i = 0; i < 5; i++)
    {
        returned_pid = fork(); // creating children using the fork 
        
        if (returned_pid < 0)
        {
            printf("That did not work. Terminate\n"); 
        }
        
        else if(returned_pid == 0) // Child process successfully created
        {
            printf("Child created with PID: %d, Born to Parent with PID: %d\n\n", getpid(), getppid());
            signal(SIGINT, staticSignal); // registers SIGINT to the function created called staticSignal
            while(1) sleep(1);  //goes into infinite loop 
            exit(0); 

        }
        else //parent process
        {
            kidspids[i] = returned_pid; // store the childs PID into the array 
        }
        
    }
    
    //COUNTDOWN FOR 5 SECONDS 
    countdown(); 
    
    // ATTEMPT 1:  (IGNORED)
    //killProcess(returned_pid, kidspids); 
    if(returned_pid > 0)
    {
    for(int i = 0; i < 5; i++)
    {
        kill(kidspids[i],SIGINT); 
    }
    
    }

    
    //ANOTHER COUNTDOWN for 5 seconds
    countdown(); 
    
    //ATTEMPT 2: (HONORED)
    //killProcess(returned_pid, kidspids); 
    if(returned_pid > 0)
    {
    for(int i = 0; i < 5; i++)
    {
        kill(kidspids[i],SIGINT); 
    }
    
    }
    exit(0); 

}
