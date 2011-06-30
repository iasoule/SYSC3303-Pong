/* Game controller component with proxy
** Idris Soule, Michael Pang
*/


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define DEFAULT_PORT     51223U
#define DEFAULT_THREADS  4U
#define DEFAULT_TASKS    20U

char model_ip_addr[15]; //TODO HEAP ALLOC

/* struct for the player essentially the client 
   holds the sockaddr_in and the current connected
   socket of this player ... easily extendable
 */
typedef struct {
    struct sockaddr_in client;
    unsigned int socket;
} player_t;

/**
  Handle client messages
  @param: arg - the clients socket
 */
void *handle_client(void *arg)
{
    player_t player = *(player_t*)&arg;
    char buffer[BUFSIZ]; 
    memset(buffer, 0, sizeof(buffer));
    for(;;){
        int status;
        status = recv(player.socket, buffer, BUFSIZ, 0);
        
        if(status == 0){
            printf("%s disconnected\n",
                    inet_ntoa(player.client.sin_addr),"l");
            pthread_exit(NULL);
        }

        else if(status < 0) {
          switch(status){
              case ECONNRESET:
                fprintf(stderr, "Client forced disconnection\n");
              break;

              case ETIMEDOUT:
                fprintf(stderr, "Connection timedout\n");
              break;

              case EIO:
                fprintf(stderr, "I/O Error\n");
              break;

              default:
                fprintf(stderr, "%s\n", strerror(errno));
              break;
          }
          pthread_exit(NULL);
        }
        /* have data on the socket 
           relay this data to the model
        */
        else {
            printf("sending data to the model...\n");
            //also have to listen for data from the model
        }
    }
}
		
void usage(char *pname)
{
     fprintf(stderr,
	    "Usage: %s -p <port> -i <model ip address>\n",
	    pname);
}

int main(int argc, char **argv)
{
    int serversock, clientsock, c;
    int clientlen; 
    unsigned short port = DEFAULT_PORT;
    struct sockaddr_in server_addr, client_addr;
    
    const char *banner =
                "**********************************"
                "\t%% Proxy started on port %u %%\n"
                "**********************************";

    while((c = getopt(argc, argv, "p:i")) != -1){
      switch(c){
        case 'p':
          port = (unsigned short) atoi(optarg);
          if (!port)
            errx(EXIT_FAILURE, "illegal port");
        break;

        case 'i':
         strcpy(model_ip_addr, optarg);
         //model_ip_addr[14] = '\0'; //safety
        break;
    
        default:
         usage(*argv);
        break;
      }
    }

    /* socket creation and structure initialization*/
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
	    errx(EXIT_FAILURE, "socket()", strerror(errno));
   
    memset(&server_addr, 0, sizeof(server_addr));	
    server_addr.sin_family = AF_INET;	/* Internet/IP */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);	/* server port */
    
    if (bind(serversock, (struct sockaddr *) &server_addr, 
                sizeof(server_addr)) < 0) 
	    errx(EXIT_FAILURE, "bind():", strerror(errno));
    /* listens for a maximum of DEFAULT_TASKS */
    if (listen(serversock, DEFAULT_TASKS) < 0)
       errx(EXIT_FAILURE, "listen():", strerror(errno));
    
    printf(banner, port);
    printf("Waiting for connections ...\n");
    
    //processing Loop
    while(true)
    {
        clientsock = accept(serversock,
                (struct sockaddr *)&client_addr, 
                (socklen_t *)&clientlen);
        
        player_t player; 
        player.client = client_addr;
        player.socket = clientsock;

        //spawn worker thread
        pthread_t worker_thread;
        if(pthread_create(&worker_thread, NULL, handle_client,
                          (void*)player))
            errx(EXIT_FAILURE, "pthread_create():",strerror(errno));
        //TODO timeouts
    }

}
