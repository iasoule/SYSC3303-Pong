#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <native/task.h>
#include <native/mutex.h>
#include <native/alarm.h>
#include <sys/mman.h>
#include <native/task.h>
#include <native/sem.h>

    
RT_TASK control_thread;
#define TASK_PRIO  99              /* Highest RT priority */
#define TASK_MODE  T_FPU|T_CPU(0)  /* Uses FPU, bound to CPU #0 */
#define TASK_STKSZ 4096            /* Stack size (in bytes) */


#define SEM_INIT 1       /* Initial semaphore count */
#define SEM_MODE S_FIFO  /* Wait by FIFO order */
RT_SEM sem_desc;
#define SEM_INIT 1 
   
 int connection = 0;

#include<stdio.h>

pid_t  pid;

void  childProcess(void)
{ 		  
    	       
}

void  ChildProcess2(void)
{ 		  
    	system("./Model2 -b 5 &");
}

int main(int argc, char *argv[])
{
	
    mlockall(MCL_CURRENT | MCL_FUTURE);
    
    int sock, connected, bytes_recieved;  
   char send_data [1024] , recv_data[1024]; 
   char sysc_call[1000];
   struct sockaddr_in server_addr,client_addr;    
   int sin_size;
  
   int port_data = 1029;
   int viewport_data = 2029;

   int err;

    /* Create a semaphore; we could also have attempted to bind to
       some pre-existing object, using rt_sem_bind() instead of
       creating it. */

    err = rt_sem_create(&sem_desc,"sendSem",SEM_INIT,SEM_MODE);


   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket");
            exit(1);
     }

   
        
      server_addr.sin_family = AF_INET;         
      server_addr.sin_port = htons(1026);     
      server_addr.sin_addr.s_addr = INADDR_ANY; 
      bzero(&(server_addr.sin_zero),8);       /* Initial semaphore count */


      if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))
                                                                       == -1) {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(sock, 5) == -1) {
            perror("Listen");

            exit(1);
        }
		pid = fork();
			if (pid == 0){
                 	      
			}        
   		 
        while(1)
        {  
            if (pid != 0){
	
            	sin_size = sizeof(struct sockaddr_in);
            	connected = accept(sock, (struct sockaddr *)&client_addr,&sin_size);
            	

            	connection++;

			

            	printf("\n I got a connection from (%s , %d)",
            	       inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
    				    
            	sprintf(send_data, "%d", port_data);
            	rt_sem_p(&sem_desc,TM_INFINITE);
	    		send(connected, send_data,strlen(send_data), 0);  
            	rt_sem_v(&sem_desc);
                if (connection==1){
                    pid = fork();
     		 	 if (pid == 0){
                             sleep(3);
                 	     sprintf(sysc_call, "./Model -p %d -b 5 -v %d &",port_data,viewport_data);
       		 	     system(sysc_call);
			}
		} 
                else if(connection %2 ==0){
            	    port_data++;
                    viewport_data++;
            	    pid = fork();
     		 	 if (pid == 0){
                             sleep(3);
                 	      sprintf(sysc_call, "./Model -p %d -b 5 -v %d &",port_data,viewport_data);
       		 	     system(sysc_call);
			}
                    
            	}
	    }
        }       

      close(sock);

}

void cleanup_module (void)

{
    rt_task_delete(&control_thread);
}
