// Pipes Task 2 
// Author Athulya Ganesh 
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h> 

int main(int argc, int ** argv) 
{ 
    pid_t child;
    char inbuf, c; 
    int p[2], i; 
    int status; 
    FILE * fp, * op; 

    if (pipe(p) < 0) exit(1);  //checking if pipe has been created.
    
    if (argc > 1)
    {
	op = fopen(argv[1],"w+"); //creates an output file with required name and write permissions. + creates the file if it doesnt exist. 
	if (op == NULL)
	{
		fprintf(stderr, "ERROR - FILE %s NOT FOUND.\n", argv[1]); 
		exit(1);
	}
    } 

    child = fork(); //creating a child 


    if (child == 0) //child code 

       { close(p[1]);   // Close down the "write" side of the pipe because the child is not writing
  
  	 while(read(p[0], &inbuf, 1))
	{
		fputc(inbuf, op); //while there are still characters to read in the childs pipe, write the characters into the file.
	}
	 fclose(op);     //close the file once done with file operations    
         // Well... the child process is done, so exit
         exit(0);
       }

    // Here's the parent code.  The parent wants to send data to the child, so it does
    else 
       { close(p[0]);   // Close the read side of the pipe, because the parent is not reading
		
	 fp = stdin;  //taking input 
	 while(!feof(fp)) //while not end of file
	{
		c = getc(fp);
		putc(c, stdout); //print to terminal 
		write(p[1], &c, 1); // Writing one character a time of the output of the command into the childs read pipe. 
		
	}

         close(p[1]);
         
         // Now the parent waits around for its child to do its work, and only after the
         // child is done will it terminate.
         waitpid(child, &status, 0);
       }
}
