/*
   Idris Soule
   Group 09
   Server.c
*/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <time.h> //post-processing
#include <signal.h>


#define MAXPENDING 5U		/* Max connection requests */
#define DEFAULT_PORT   1026U

FILE *Log = NULL;
bool loggable = false;
const char *client_ip = NULL;

//incase of CTRL-C close the Log 
//can be extended for further cleanup
void sig_hdlr(int sig)
{
    if(Log)
      fclose(Log);
    fprintf(stderr, "Server exiting ...\n");
}

void usage(char *pname)
{
   static const char *fmt_error = 
        "Usage: %s -l <optional log>\n"
        "\t-l: logs the clients data to file\n"
        "\t-h: help\n"
        "By the spec server starts on port 1026";
    fprintf(stderr, fmt_error, pname);
    exit(0);
}

bool is_telnet(char *str, size_t len, int *pos)
{
    int i = 0;
    while(len--){
        if(str[i] == (char)0xD){//CR is never w/o LF 
            *pos = i;
            return true;
        }
    }
    return false;
}

void handle_request(int sock)
{
    char buffer[BUFSIZ];
    int received = -1;
    time_t curr_time; 
    struct tm *timeinfo;

    while(!strstr(buffer, "{EXIT}")){
    memset(buffer, 0, sizeof(buffer));
    if ((received = recv(sock, buffer, BUFSIZ, 0)) < 0) 
	  errx(EXIT_FAILURE, "recv():",strerror(errno));
    
    printf("%s\n", buffer);
    /* check for telnet 
       -- remember that there will be no NULL terminator
    */
    int pos;
    if (is_telnet(buffer, sizeof(buffer), &pos))
        buffer[pos] = '\0'; //terminate cleanly for printf
      
    if(loggable){
        time(&curr_time);
        timeinfo = localtime(&curr_time);
        fprintf(Log,"*------------------------------*\n");
        fprintf(Log, "%% %s", asctime(timeinfo));
        fprintf(Log, "      From: %s\n", client_ip);
        fprintf(Log, "      Command: %s\n", buffer);
        fprintf(Log,"*------------------------------*\n\n");
    }
    }
    close(sock); //received {EXIT}
}

int main(int argc, char **argv)
{
    int serversock, clientsock;
    struct sockaddr_in echoserver, echoclient;
    int c;

    /* setup signalling incase of CTRL-C */
    struct sigaction ctrl_act;
        ctrl_act.sa_handler = sig_hdlr;
        sigemptyset(&ctrl_act.sa_mask);
        ctrl_act.sa_flags = 0; 
    sigaction(SIGINT, &ctrl_act, 0); 

    while((c = getopt(argc, argv, "hl:")) != -1){
	switch(c) {
	 case 'l':
	 case 'L':;
	  	char *log_name = optarg;
	    if ((Log = fopen(log_name, "w")) == NULL)
		   errx(EXIT_FAILURE, "fopen():", strerror(errno));
        loggable = true;
        fprintf(Log,"+------------------------------+\n");
        fprintf(Log,"\tGAME SERVER LOG\n");
        fprintf(Log,"+------------------------------+\n");
  	 break;

	 case 'h':
	 case 'H':
	 default:
	    usage(*argv);
	 break;
	}
    }

    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	    errx(EXIT_FAILURE,"socket():",strerror(errno));
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));	/* Clear struct */
    echoserver.sin_family = AF_INET;	/* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);	/* Incoming addr */
    echoserver.sin_port = htons(DEFAULT_PORT);	/* server port */
    
    if (bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0)
	    errx(EXIT_FAILURE,"bind():",strerror(errno));
  
    if (listen(serversock, MAXPENDING) < 0) 
	   errx(EXIT_FAILURE, "listen():", strerror(errno));

    

    while (1) {
	unsigned int clientlen = sizeof(echoclient);
	/* Wait for client connection */
	if ((clientsock =
	     accept(serversock, (struct sockaddr *) &echoclient,
		    &clientlen)) < 0) {
	    errx(EXIT_FAILURE, "accept():", strerror(errno));
	}
	client_ip = inet_ntoa(echoclient.sin_addr);
    printf("Client connected: %s\n", client_ip);
	handle_request(clientsock);
    }
    fclose(Log); //clean up resources
}
