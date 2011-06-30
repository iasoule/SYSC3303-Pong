/*
   Group 09 
   Idris Soule, MPang
   Client.c
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <err.h>
#include <stdbool.h>
#include <ctype.h>

#define DEFAULT_PORT    1026U

       
pid_t  pid;
FILE *Log, *Cmd;
bool loggable = false, read_cmd = false, sendcomplete = false;
const char *server_ip = NULL;

#define S_BUFLEN              25

#define arrow_step_one  27
#define arrow_step_two  91
#define arrow_right  67
#define arrow_left  68

#define start  10
#define add  32
#define exit_game  113


#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
pid_t  pid;

 int view_port;
    char sysc_call[1000];
static struct termios oldt;

void restore_terminal_settings(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void disable_waiting_for_enter(void)
{
    struct termios newt;

    /* Make terminal read 1 char at a time */
    tcgetattr(0, &oldt);  /* Save terminal settings */
    newt = oldt;  /* Init new settings */
    newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
    tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
    atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}

/**
  parses the file and executes commands 
  from them based on the macro-language 
  (going to assume the file is proper!)
 */
void parse_file(int sock)
{
    int c;
    bool in_number = false;
    char command[BUFSIZ];
    unsigned int echolen = 0U;
    struct tm *timeinfo; 
    time_t curr_time;

    while(true){ //parse the file  
        fgets(command, BUFSIZ, Cmd);
        if(strstr(command, "#END")) //EOF-MARKER for portability
            break;
        in_number = command[0] == '[';       
        echolen = strlen(command);
        if(!in_number){
           if(send(sock, command, echolen, 0) != echolen)
	           errx(EXIT_FAILURE, "::send():", strerror(errno));
           
           printf("reading %s from file\n", command);
           if(loggable){
             time(&curr_time);
             timeinfo = localtime(&curr_time);
             fprintf(Log,"*------------------------------*\n");
             fprintf(Log, "%% %s", asctime(timeinfo));
             fprintf(Log, "      To: %s\n", server_ip);
             fprintf(Log, "      Command: %s\n", command);
             fprintf(Log,"*------------------------------*\n\n");
          }
        }
        else {
            /* timeout, since C defines
               2 functions at the us and s level use a little math
               also clear out the square brackets for sscanf
            */
            float tsec;
            command[strlen(command)-1] = '\0'; //clean ']'
            sscanf(&command[1], "%f", &tsec);

            printf("delaying %fs ...\n", tsec);
        pid_t  pid;    sleep((int)tsec);
            usleep(1000U * (float)(tsec - (int)tsec));
            puts("\npreparing to send ...\n");
        }
        memset(command, 0, sizeof(command));
    }
    fclose(Cmd);
}

// put string in upper case
void strupr(char *str)
{
    size_t i = 0; 
    while(str[i] != '\0'){
        str[i] = toupper(str[i]);
        i++;
    }
}

void usage(char *pname)
{
	printf("Usage: %s -i <server ip> -l <log file> -r <command file>\n", pname);
	printf("\t-i: the game server\n");
	printf("\t-l: optional logfile\n");
	printf("\t-r: read from command file\n");
	printf("There are two options either read from <stdin> or a command file\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in echoserver;
    char buffer[BUFSIZ];
    unsigned int echolen;
    int received = 0, c, sock,sock2;
    int ch;
    int enableArrow =0;
    
    disable_waiting_for_enter();
    bool sendable;
    if(argc < 3)
        usage(*argv);
   while((c=getopt(argc, argv, "l:r:i:h")) != -1){
	  switch(c){
          case 'L':
          case 'l':
            if((Log = fopen(optarg, "w")) == NULL)
                errx(EXIT_FAILURE, "fopen():", strerror(errno));
            loggable = true;
            fprintf(Log,"+------------------------------+\n");
            fprintf(Log,"\t  CLIENT LOG\n");
            fprintf(Log,"+------------------------------+\n");
          break;
            
          case 'I':
          case 'i':
            server_ip = optarg;
          break;

          case 'R':
          case 'r':
            if((Cmd = fopen(optarg, "r")) == NULL)
                errx(EXIT_FAILURE, "fopen():", strerror(errno));
            read_cmd = true; 
          break;
          
          case 'h':
          default:
            usage(*argv);
          break;
      }
   }

			
    /* Create the TCP socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	        errx(EXIT_FAILURE, "socket():", strerror(errno));
    printf("%s","ready to Server Launcher\n");
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));	/* Clear struct */
    echoserver.sin_family = AF_INET;	/* Internet/IP */
    echoserver.sin_addr.s_addr = inet_addr(server_ip);	/* IP address */
    echoserver.sin_port = htons(DEFAULT_PORT);	/* game launch port */
    /* Establish connection */
    if (connect(sock,
		(struct sockaddr *) &echoserver, sizeof(echoserver)) < 0)
        errx(EXIT_FAILURE, "connect():1", strerror(errno));

    printf("%s","Connected to Server Launcher\n");
    char portBuffer[BUFSIZ];
    int status;
   

    if((status = recv(sock, portBuffer, BUFSIZ, 0)) < 0)
        errx(EXIT_FAILURE, "connect():", strerror(errno));
    
    close(sock);
 
    printf("%s","Ready to Model\n");
     /* Create the TCP socket */
    if ((sock2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	        errx(EXIT_FAILURE, "socket():", strerror(errno));
  
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));	/* Clear struct */
    echoserver.sin_family = AF_INET;	/* Internet/IP */
    echoserver.sin_addr.s_addr = inet_addr(server_ip);	/* IP address */
    echoserver.sin_port = htons((short)atoi(portBuffer));	 /* server port */
    puts(portBuffer);
     pid = fork();
    if (pid == 0){
         view_port= 2029+atoi(portBuffer) - 1029;
   	 sprintf(sysc_call, "./View %d",view_port);
   	 system(sysc_call);
    }   
  
    printf("%d \n",view_port);

    sleep(5);
    if (connect(sock2,
		(struct sockaddr *) &echoserver, sizeof(echoserver)) < 0)
        errx(EXIT_FAILURE, "connect():2", strerror(errno));
    printf("%s","Connected to Model");

    /* determine whether we read from file or stdin */
    FILE *stream;
    stream = read_cmd ? Cmd : stdin;
       
    if(stream == stdin){ //commandline mode
        while(strcmp(buffer, "EXIT")){
            memset(buffer, 0, sizeof(buffer));
            //fgets(buffer,BUFSIZ,stdin);
           ch = getchar();
            if(strlen(buffer) > 0)
                buffer[strlen(buffer)-1] = '\0'; //no newline
            /*strupr(buffer);
            echolen = strlen(buffer);
            //write data to game server
	    */
	
	
           if(ch == arrow_step_one){
                enableArrow++;
	    }
 	   else if(ch == arrow_step_two && enableArrow==1){
                 enableArrow++;
	
		}       
           else if(ch == arrow_left && enableArrow==2){
 		
                 strcpy(buffer,"LEFT");
		 printf(">>%s\n",buffer);
		
                 echolen = strlen(buffer);
		
		 enableArrow =0;
		 sendable= true;
		
            }
           else if(ch == arrow_right && enableArrow==2){
                 strcpy(buffer,"RIGHT");
		 printf(">>%s\n",buffer);
                 echolen = strlen(buffer);
		 enableArrow =0;
		
		 sendable= true;
		
            }     
	
           else
  		enableArrow =0;



             if(ch == start){          
 		strcpy(buffer,"START");
		printf(">>%s\n",buffer);
                echolen = strlen(buffer);
		
                sendable= true;
	     }
	     if(ch == add){            
 		strcpy(buffer,"ADD");
		printf(">>%s\n",buffer);
                echolen = strlen(buffer);
		
                sendable= true;
	     }	 
	    if(ch == exit_game){            
 		strcpy(buffer,"EXIT");
		printf(">>%s\n",buffer);
                echolen = strlen(buffer);
	
                sendable= true;
                
	     }
	  if(sendable){
	
		
            if(send(sock2, buffer, echolen, 0) != echolen)
	            errx(EXIT_FAILURE, "send():", strerror(errno));

	
            if(loggable){
               struct tm *timeinfo;
               time_t curr_time;
               time(&curr_time); timeinfo = localtime(&curr_time);
               fprintf(Log,"*------------------------------*\n");
               fprintf(Log, "%% %s", asctime(timeinfo));
               fprintf(Log, "      To: %s\n", server_ip);
               fprintf(Log, "      Command: %s\n", buffer);
               fprintf(Log,"*------------------------------*\n\n");
            }
	  }
	  sendable = false; 
        } 
    }
    //read from file
    else {
        parse_file(sock);
    }

    /* cleanup */
      close(sock);
      fprintf(stdout, "\nSockets closed and resources deallocated\n");
      fprintf(stdout, "Exiting ...\n");
      exit(0);

}
