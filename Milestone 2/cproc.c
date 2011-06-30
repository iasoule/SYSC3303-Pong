/* Milestone 2 
 ** Idris Soule, Michael Pang
   C program starting Java process
*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

//#define DEBUG 0

int main(int argc, char **argv)
{
   const char *jarFile = "PongGame.jar";

   if(argc != 2){
     fprintf(stderr, "Usage: %s <program name>\n",*argv);
     return -1;
   }
   
   //start the Java process
   int pid; 
   pid = fork();
   switch(pid){
      case -1:
        perror("fork() failed");
  	exit(-1);
      break;
    	
      case 0: 
        //executes the jar
        #if DEBUG 
        jarFile = argv[1];
        #endif
	if(execl("/usr/bin/java", "/usr/bin/java", "-jar",jarFile) < 0){
           fprintf(stderr, "Error opening jar file\n%s", strerror(errno));
           exit(-1);
	}
      default:
	wait();
      break;
   }
   exit(0);
   }
