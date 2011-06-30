/**
   Idris Soule , Michael Pang
   SYSC 3303 Milestone 2
**/

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

#include <native/task.h>
#include <native/mutex.h>
#include <native/alarm.h>
#include <sys/mman.h>


#define VIEW_DEFAULT_PORT     1027U
#define MODEL_DEFAULT_PORT    1026U
#define S_BUFLEN              25
#define BTHREAD_RATIO         1

#define rt_err(__xrt_err_str__) \
 errx(!EXIT_SUCCESS, __xrt_err_str__":", strerror(-errno))


#define padString(s) \
do { \
   int _pad_s = 0; \
   for(;_pad_s < S_BUFLEN; _pad_s++) \
      if(s[_pad_s] == (char)0)\
	s[_pad_s] = 'x'; \
}while(0)

RT_TASK ballThread;
RT_TASK sendThread, recvThread[2];
RT_TASK alarmTask;

RT_MUTEX autoStart;
RT_MUTEX txMutex[3]; //transmission mutex on buffer(s) send/recieve
enum { E_GAME, E_ADD, E_MOVE }; //enums for MUTEX indices

char gameData[S_BUFLEN], pongData[2][S_BUFLEN];
char moveData[2][S_BUFLEN],  ballData[S_BUFLEN]; //no contention for ballData 1 thread access it only

void run(void *vsocket);

struct Point{
    int point_x;
    int point_y;
    int radius;
};
struct pongWidget{

    struct Point leftCorner;
    int length;
    int height;
};

struct BallWidget{
    struct Point centre;
    int radius;
    bool created;
};


const  int LENGTH_DELTA   = 40;
const  int DEFAULT_RADIUS = 20;

int resolution_x = 0;
int resolution_y = 0;
int resolution_w = 600;
int resolution_h = 600;

//Pong Class

#define MAX_PONGS_PER_SIDE  1
#define MAX_PONGS           2
#define DEFAULT_HEIGHT      467
#define DEFAULT_BALLSPEEDX  0
#define DEFAULT_BALLSPEEDY  (-1)

typedef struct { 
      int socket;
      int vsocket; //view socket
      struct sockaddr_in addr;
} player_t;

typedef struct {
	 player_t *client;
         char *buffer;
}send_packet_t;


//1st player connected (for point state)
int       initSock = 0; 
const int player   = 1; 
const int opponent = 0;

const int pongArea[] = {10,20,30,50,60,70,80};

const int RlargeAngleBounce =  3,  RnormalAngleBounce = 2;
const int RsmallAngleBounce =  1,  noAngleBounce = 0;
const int LlargeAngleBounce = -3,  LnormalAngleBounce = -2;
const int LsmallAngleBounce = -1;

const int standardPongSpeed = 10;

unsigned int player_score =0;
unsigned int opponent_score =0;

bool reset        = false;
bool bounceplayer = false, bounceOpponent = false;

int locationPointOfX;
int oppoentPointLocationX;

int currentBallOfX;
int currentBallOfY;

int defaultBalllocationX;
int defaultBalllocationY;

int balldirectionX;
int balldirectionY;

unsigned int ballThreadSpeed;

bool AIMode = false;
bool issuedStart = true; // force issue the start command or not

struct pongWidget pongs[2];
struct BallWidget ball;
int nPongs;
int pongLength;

void initializePong(){
    pongLength = MAX_PONGS * LENGTH_DELTA;
    pongs[0].height = 10;
    pongs[1].height = 10;
    pongs[0].leftCorner.point_x = 260;
    pongs[0].leftCorner.point_y = 0;
    pongs[1].leftCorner.point_x = 260;
    pongs[1].leftCorner.point_y = DEFAULT_HEIGHT;
    ball.centre.point_x = 300;
    ball.centre.point_y = 300;
    ball.radius         = 20;
    ball.created = false;
    balldirectionX = 0;
    balldirectionY = -1;
}

//original function in java call reset
void resetBall(){
    if(ball.created){
        ball.centre.point_x = defaultBalllocationX;
        ball.centre.point_y = defaultBalllocationY;
        balldirectionX = DEFAULT_BALLSPEEDX;
        balldirectionY = DEFAULT_BALLSPEEDY;
	reset = false;
    }

    /* reset the AI opponent */
    if(AIMode){
	pongs[opponent].leftCorner.point_x = 260;
    }
}

int illegal_move_check (int checkLocation, int checkOutOfBounds){
     int locationOutOfBounds;
   
  locationOutOfBounds = checkLocation + checkOutOfBounds + standardPongSpeed + pongLength;
     
     if(locationOutOfBounds  > resolution_w) //R
	  return 1;
     else if (locationOutOfBounds-pongLength == 2*standardPongSpeed) //L
	  return -1;
     else 
	  return 0;
}

void changeBallDirectionHorizontal(){
    balldirectionX *= -1;
}

void changeBallDirectionVertical(){
    balldirectionY *= -1;
}

/* generate computer moves */      
char AImoveGenerate()
{
   int n;
   srand((unsigned int)time(NULL));
   n = 20 * rand() / (RAND_MAX + 1.0); 
   
   return n % 2 == 0 ? 'R' : 'L';
}

void bouncingCheck (int x,int y, int radius, int directionX,int directionY){
        if(x + directionX < resolution_x){
            changeBallDirectionHorizontal();
        }
        else if(x +directionX + radius > resolution_w){
            changeBallDirectionHorizontal();
        }
        /* check if we hit the topmost pong */
        if (y+ directionY + (radius/4) <pongs[opponent].leftCorner.point_y + pongs[opponent].height ){
             if(x+directionX + radius > pongs[opponent].leftCorner.point_x &&
                x+directionX + radius < pongs[opponent].leftCorner.point_x+pongLength+radius){
                    bounceOpponent = true;
                    changeBallDirectionVertical();
                }
             else{ // in the vicinity to score a point 
                 player_score++;
		 rt_mutex_acquire(&txMutex[E_GAME],TM_INFINITE);
		 memset(gameData, 0, sizeof(gameData));
		 sprintf(gameData, "score;%d;%d;",player_score,opponent_score);
		 padString(gameData);
		 rt_mutex_release(&txMutex[E_GAME]);                 
                 reset = true;
             }
            
         }
	
        /* check if we hit the bottom-most pong */  
        if (y+ directionY + (radius/4) +pongs[player].height >pongs[player].leftCorner.point_y){
	     if(x+directionX + radius > pongs[player].leftCorner.point_x &&
                x+directionX + radius < pongs[player].leftCorner.point_x+pongLength+radius){
                    bounceplayer = true; 
                    changeBallDirectionVertical();
                }
	     else{                  opponent_score++;
                 rt_mutex_acquire(&txMutex[E_GAME],TM_INFINITE);
 		 memset(gameData, 0, sizeof(gameData));
		 sprintf(gameData, "score;%d;%d;",player_score,opponent_score);
		 padString(gameData);
		 rt_mutex_release(&txMutex[E_GAME]); 
                 reset = true;
             }
             
       }
		
}

void bounceDirection(int pongIdentifier, int radius){
    int tempBallX = currentBallOfX + balldirectionX;
    int area[7], i;
    int angles[] = { LnormalAngleBounce, LsmallAngleBounce, noAngleBounce,
                     RsmallAngleBounce, RnormalAngleBounce,  RlargeAngleBounce
 		   };
                                                       
    for(i = 0; i < 7; i++) 
      area[i] = pongs[pongIdentifier].leftCorner.point_x + pongArea[i];
   
    if(currentBallOfX + radius >= pongs[pongIdentifier].leftCorner.point_x 
				&& tempBallX < area[0]){
        balldirectionX = LlargeAngleBounce;
    }
    else {
       for(i = 0; i <= 5; i++) 
        if (tempBallX >= area[i] && tempBallX < area[i+1]){
           balldirectionX = angles[i];
           break;
        } 
    }
}


void widgetNotification(struct pongWidget p_widget){
    pongs[nPongs++] = p_widget;
}


void addBall(){

    if(!ball.created){ //singleton pattern
        ball.centre.point_x = (resolution_w-resolution_x)/2;
        ball.centre.point_y = (resolution_h-resolution_y)/2;
        defaultBalllocationX = (resolution_w-resolution_x)/2;
        defaultBalllocationY =  (resolution_h-resolution_y)/2;

	currentBallOfX = ball.centre.point_x;
        currentBallOfY = ball.centre.point_y;

        // pPanel notify for created pPanel
        balldirectionX = DEFAULT_BALLSPEEDX;
        balldirectionY = DEFAULT_BALLSPEEDY;
        
	rt_mutex_acquire(&txMutex[E_GAME], TM_INFINITE);
	memset(gameData, 0, sizeof(gameData));
	sprintf(gameData, "score;%d;%d;",player_score,opponent_score);
	padString(gameData);
	rt_mutex_release(&txMutex[E_GAME]);
      	
	 memset(ballData, 0, sizeof(ballData));
         sprintf(ballData, "ball;%d;%d;%d;",defaultBalllocationX, defaultBalllocationY, ball.radius);
         padString(ballData);
	 ball.created = true;
    }
}

void resetScore(){
        if(nPongs >= MAX_PONGS){
            player_score = 0;
            opponent_score = 0;
            rt_mutex_acquire(&txMutex[E_GAME], TM_INFINITE);
	    memset(gameData, 0, sizeof(gameData));
  	    sprintf(gameData, "reset;");
	    padString(gameData);
	    rt_mutex_release(&txMutex[E_GAME]);
        }
}
void start() {    
      addBall();
}

void addPong(){
    if (nPongs < MAX_PONGS){
        pongs[nPongs].leftCorner.point_x = ((resolution_w-resolution_x)/2 - (pongLength/2));
        pongs[nPongs].leftCorner.point_y = resolution_y;
        oppoentPointLocationX = ((resolution_w-resolution_x)/2 - (pongLength/2));

        rt_mutex_acquire(&txMutex[E_ADD], TM_INFINITE);
	memset(pongData[0], 0, sizeof(pongData[0]));
        sprintf(pongData[0], "pong;%d;",pongs[nPongs].leftCorner.point_x);
        sprintf(pongData[0] + strlen(pongData[0]), 
		"%d;%d;%d;", pongs[nPongs].leftCorner.point_y,pongLength,pongs[nPongs].height);
	padString(pongData[0]);
        rt_mutex_release(&txMutex[E_ADD]);
        
        pongs[nPongs].length = pongLength;
        nPongs++;
        pongs[nPongs].leftCorner.point_x =((resolution_w-resolution_x)/2 - (pongLength/2));
        pongs[nPongs].leftCorner.point_y = DEFAULT_HEIGHT;
        locationPointOfX = ((resolution_w-resolution_x)/2 - (pongLength/2)); 

	rt_mutex_acquire(&txMutex[E_ADD], TM_INFINITE);
	memset(pongData[1], 0, sizeof(pongData[1]));
        sprintf(pongData[1], "pong;%d;",pongs[nPongs].leftCorner.point_x);
        sprintf(pongData[1] + strlen(pongData[1]), 
		"%d;%d;%d;", pongs[nPongs].leftCorner.point_y,pongLength,pongs[nPongs].height);
	padString(pongData[1]);
	rt_mutex_release(&txMutex[E_ADD]);
        nPongs++;
    }
}


void move(char str){
     
    int err;
    if(str == 'R'){
          if ((err=illegal_move_check(locationPointOfX, standardPongSpeed)) == 0){
              locationPointOfX = locationPointOfX + standardPongSpeed;
              pongs[player].leftCorner.point_x = locationPointOfX;
              pongs[player].leftCorner.point_y = DEFAULT_HEIGHT;
	  }
          else {
	        locationPointOfX = locationPointOfX - (err * standardPongSpeed);
	        pongs[player].leftCorner.point_x = locationPointOfX;
          }
    }

    else if (str == 'L'){
        if ((err=illegal_move_check(locationPointOfX, standardPongSpeed)) == 0){
              locationPointOfX = locationPointOfX - standardPongSpeed;
              pongs[player].leftCorner.point_x = locationPointOfX;
              pongs[player].leftCorner.point_y = DEFAULT_HEIGHT;
          }
	else {
	      locationPointOfX = locationPointOfX + (-err * standardPongSpeed);
	      pongs[player].leftCorner.point_x = locationPointOfX;
	  }
    }

       rt_mutex_acquire(&txMutex[E_MOVE], TM_INFINITE);
       memset(moveData[0], 0, sizeof(moveData[0]));
       sprintf(moveData[0], "player1;%d;%d;%d;%d;",
               pongs[player].leftCorner.point_x, pongs[player].leftCorner.point_y,pongLength, 
               pongs[player].height);
       padString(moveData[0]);
       rt_mutex_release(&txMutex[E_MOVE]);
}

void opponentMovement(char strOpponent){
    int err;

    if(strOpponent == 'R'){
          if (!(err=illegal_move_check(oppoentPointLocationX, standardPongSpeed))){
              oppoentPointLocationX = oppoentPointLocationX + standardPongSpeed;
              pongs[opponent].leftCorner.point_x = oppoentPointLocationX;
              pongs[opponent].leftCorner.point_y = resolution_y;
          }
	   else {
	        oppoentPointLocationX = oppoentPointLocationX - (err * standardPongSpeed);
	        pongs[opponent].leftCorner.point_x = oppoentPointLocationX;
          }
    }

    else if (strOpponent == 'L'){
        if (!(err=illegal_move_check(oppoentPointLocationX, standardPongSpeed))){
              oppoentPointLocationX = oppoentPointLocationX - standardPongSpeed;
              pongs[opponent].leftCorner.point_x = oppoentPointLocationX;
              pongs[opponent].leftCorner.point_y = resolution_y;
          }
	else {
	      oppoentPointLocationX = oppoentPointLocationX + (-err * standardPongSpeed);
	      pongs[opponent].leftCorner.point_x = oppoentPointLocationX;
	  }
    }

     rt_mutex_acquire(&txMutex[E_MOVE], TM_INFINITE);
     memset(moveData[1], 0, sizeof(moveData[1]));
     sprintf(moveData[1], "player2;%d;%d;%d;%d;",
         pongs[opponent].leftCorner.point_x, pongs[opponent].leftCorner.point_y,pongLength, pongs[opponent].height);
     padString(moveData[1]);
     rt_mutex_release(&txMutex[E_MOVE]);
}
 
void ballmove()
{	
   currentBallOfX = ball.centre.point_x;
   currentBallOfY = ball.centre.point_y;

   bouncingCheck(currentBallOfX, currentBallOfY, ball.radius, balldirectionX, balldirectionY);
   currentBallOfX += balldirectionX;
   currentBallOfY += balldirectionY;
   ball.centre.point_x = currentBallOfX;
   ball.centre.point_y = currentBallOfY;

   memset(ballData, 0, sizeof(ballData));
   sprintf(ballData, "ball;%d;%d;%d;", currentBallOfX, currentBallOfY, ball.radius);
   padString(ballData);
}


/* run : worker thread for ball control 
         along with AI generation
*/
void run(void *vsocket){
    int sock = *(int *)&vsocket;
    int echolen;
    RTIME sleep; // as nano-seconds


    if(ballThreadSpeed % 6 == 0)
	ballThreadSpeed++;
    sleep = (RTIME)(50000000 / (ballThreadSpeed % 6));

    rt_task_suspend(NULL);
    while (true){
   	ballmove();
	echolen = strlen(ballData);
	if(send(sock, ballData, echolen, 0) != echolen)
		rt_err("send()");
        
	if(AIMode){
            opponentMovement(AImoveGenerate());
	    echolen = strlen(moveData[1]);
	   if(send(sock, moveData[1], echolen, 0) != echolen)
		rt_err("send()");
        }
        if(reset){
            resetBall();
            echolen = strlen(gameData);
            if(send(sock, gameData, echolen, 0) != echolen)
                   rt_err("send()");
        }
        if(bounceplayer){
            bounceDirection(player,ball.radius);
            bounceplayer = false;
        }
        if(bounceOpponent){
            bounceDirection(opponent,ball.radius);
            bounceOpponent = false;
        }
        rt_task_sleep(sleep);
    }
}


void recv_data(void *arg)
{
   player_t *client = (player_t *)arg;
   char buffer[BUFSIZ];
   unsigned int echolen;
   
   rt_task_suspend(NULL); //will wait on main to signal

   for(;;){
      memset(buffer,0,sizeof(buffer));
      int status;

      if((status = recv(client->socket, buffer, BUFSIZ, 0)) < 0)
         rt_err("recv()");
      
     /* player cannot start until the given time */
      if(!strcmp(buffer,"START")){
        start(); 
        /* score initialization from start()*/
        echolen = strlen(gameData);
        if(send(client->vsocket, gameData, echolen, 0) != echolen)
     		rt_err("send()");

	/* Ball initial position */
	echolen = strlen(ballData);
	if(send(client->vsocket, ballData, echolen, 0) != echolen)
		rt_err("send()");

       	/* ball Thread can now run */
	if(rt_task_resume(&ballThread))	
	   rt_err("rt_task_resume()");
  	continue;
      }
      else if(!strcmp(buffer,"RESET")){
	resetScore();
        echolen = strlen(gameData);
	if(send(client->vsocket, gameData, echolen, 0) != echolen)
	      rt_err("send()");
	continue;
      }
      else if(!strcmp(buffer, "ADD")){
        addPong();
        echolen = strlen(pongData[0]);
	if(send(client->vsocket, pongData[0],echolen, 0) != echolen)
     		rt_err("send()");
        echolen = strlen(pongData[1]);
	if(send(client->vsocket, pongData[1],echolen, 0) != echolen) 
     		rt_err("send()");
	continue;
      }

      /* parsing L(k) and R(k) */ 
      if(client->socket == initSock){
        if(!strcmp(buffer, "LEFT") || !strcmp(buffer, "RIGHT")){
	    move(buffer[0]); //either 'L' or 'R' -> {L/R1}
            echolen = strlen(moveData[0]);
	    if(send(client->vsocket, moveData[0],echolen, 0) != echolen)
     		rt_err("send()");
        }
      }

      else if(client->socket != initSock){
        if(!strcmp(buffer,"LEFT") || !strcmp(buffer, "RIGHT")){
	    opponentMovement(buffer[0]); // {L/R2}
            echolen = strlen(moveData[1]);
	    if(send(client->vsocket, moveData[1],echolen, 0) != echolen)
     		rt_err("send()");
	}
      }
      
      /* TODO Cases to think about 
         1. need 2 alert other players about P(X) exiting
         2. close this socket, potentially alert main that another client can connect
         3. ^ if possible pause game save state -> MILESTONE 3 
      */
      else if(!strcmp(buffer,"EXIT")){
        memset(gameData, 0, sizeof(gameData));
	strcpy(gameData, "Exit;");
	echolen = strlen(gameData);
	if(send(client->vsocket, gameData, echolen, 0) != echolen)
		rt_err("send()");
      }
    }
}

/* alarm_handler 
** is triggered once 10s is passed to inform player 
** wait time has expired - issue start
*/
void alarm_handler(void *arg) 
{
     int sock = *(int *)&arg;
     int echolen;
    
     /* start the countdown */
     rt_task_set_periodic(NULL, TM_NOW, 8000000000ULL);
     rt_task_wait_period(NULL); 

     if(!issuedStart)
       rt_task_delete(NULL); //not need two players connected

     rt_mutex_acquire(&autoStart, TM_INFINITE);
     issuedStart = true;
     rt_mutex_release(&autoStart);
     AIMode = true;
      
     printf("FORCED START client playing with AI!\n");
     /* below we emulate the add, start */
     addPong();     
     echolen = strlen(pongData[0]);
     if(send(sock, pongData[0],echolen, 0) != echolen)
     	rt_err("send()");
     echolen = strlen(pongData[1]);
     if(send(sock, pongData[1],echolen, 0) != echolen) 
     	rt_err("send()");

     start(); 
     echolen = strlen(gameData);
     if(send(sock, gameData, echolen, 0) != echolen)
     	 rt_err("send()");

     echolen = strlen(ballData);
     if(send(sock, ballData, echolen, 0) != echolen)
	 rt_err("send()");

     if(rt_task_resume(&recvThread[0]))
	rt_err("rt_task_resume()");
     if(rt_task_resume(&ballThread))	
	 rt_err("rt_task_resume()");

}

void usage(char *pname)
{
   fprintf(stderr, "Usage: %s -i <view ip> -b <ball speed>\n",pname);
   fprintf(stderr, "Xenomai model for the pong game\n"
		   "Option:\n"
		   "\t i: dotted quad of the views machine\n"
		   "\t b: difficulty for ballspeed on [1,5]\n"
		   "\t    where 1 is the Easiest\n");
} 

int main(int argc, char **argv)
{
    int serversock, clientsock;
    size_t clientlen;  
    struct sockaddr_in server_addr, client_addr, view_addr;
    unsigned int port = MODEL_DEFAULT_PORT; //models and view iff on same cpu

    char *ipAddress;
    
    mlockall(MCL_CURRENT | MCL_FUTURE);

    if(argc < 3){
	usage(*argv);
	exit(0);
    }
	
    int c;
    while((c = getopt(argc, argv, "i:b:")) != -1){
 	switch(c){
	   case 'i':
	     ipAddress = strdup(optarg);
	   break;
	  
	   /* many potential super fast speeds can be achieved */
	   case 'b':    
	   ballThreadSpeed = atoi(optarg) > 0 ?
			     atoi(optarg) : BTHREAD_RATIO;	
      	   break;
	   default:
		usage(*argv);
		exit(0);
	   break;
        }
    }

    initializePong();

    /* task creation */
    if(rt_task_create(&ballThread, "BallThread", 4096, 99, 0))
	rt_err("rt_task_create()");
    if(rt_task_create(&recvThread[0], "RecvThread", 4096, 99, T_JOINABLE))
	rt_err("rt_task_create()");    
    if(rt_task_create(&recvThread[1], "RecvThread1", 4096, 99, T_JOINABLE))
	rt_err("rt_task_create()"); 
    
    /* creation of mutexes for buffer */
    unsigned int i = 0; 
    for(; i < 3; i++){
       char buf[8];
       sprintf(buf, "Mutex %d", i);
       if(rt_mutex_create(&txMutex[i], buf))
          rt_err("rt_mutex_create()");
    }
    if(rt_mutex_create(&autoStart, "AutoStartMutex"))
       rt_err("rt_mutex_create()");

    i = 0;
     /* socket creation and structure initialization*/
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
	    errx(EXIT_FAILURE, "socket()", strerror(-errno));
   
    memset(&server_addr, 0, sizeof(server_addr));	
    server_addr.sin_family = AF_INET;	/* Internet/IP */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);	/* server port */
    
    if (bind(serversock, (struct sockaddr *) &server_addr, 
                sizeof(server_addr)) < 0){
	    close(serversock); 
	    errx(EXIT_FAILURE, "bind():", strerror(errno));
    }
    /* listens for Game Controller only*/
    if (listen(serversock, 1) < 0)
       errx(EXIT_FAILURE, "listen():", strerror(-errno));

    
    /* now need to connect to the view */
    int viewSocket; 
    if ((viewSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        rt_err("socket()");

    memset(&view_addr, 0, sizeof(view_addr));
    view_addr.sin_family = AF_INET;	
    view_addr.sin_addr.s_addr = inet_addr(ipAddress);	
    view_addr.sin_port = htons(VIEW_DEFAULT_PORT);
    /* Establish connection to view */
    if (connect(viewSocket,
		(struct sockaddr *) &view_addr, sizeof(view_addr)) < 0)
	rt_err("connect()"); 

       /* connection loop */
    printf("Waiting for connections ...\n");

    while(true){ 
      char *currClient;
        
        clientlen = sizeof(struct sockaddr_in);
        clientsock = accept(serversock,
                (struct sockaddr *)&client_addr, 
                (socklen_t *)&clientlen);
	
      currClient = (char *)inet_ntoa(client_addr.sin_addr);
      if(i > 1 || (i < 1 && !issuedStart)){ //game in play by force or 2 players
	close(clientsock); 
        fprintf(stderr, "rejecting %s "
			"only 2 clients allowed!\n", currClient);
	continue;
      }
  
      
      //spawn the receive thread for simulator
      player_t players[2];
      players[i].socket  = clientsock;
      players[i].vsocket = viewSocket;
      players[i].addr    = client_addr;
      if(rt_task_start(&recvThread[i], recv_data, (void *)&players[i]))
	rt_err("rt_task_start()");
      i++;
      
      /* 1st player connected */
      if(i == 1){ 
         initSock = clientsock; 
    
      /* spawn task to countdown */                
         if(rt_task_spawn(&alarmTask, "AutoStart", 4096, 99, 0, 
			  alarm_handler, (void *)viewSocket))
            rt_err("rt_task_spawn()");
 
         /* start the ball thread and it handles further movement */
        if(rt_task_start(&ballThread, run,(void*)viewSocket))
	  rt_err("rt_task_start()");
        printf("1 Player Connected ...\n");
       	  
      }

      /* the case when both players start before one-shot alarm */
      if(i == 2){
        rt_mutex_acquire(&autoStart, TM_INFINITE);
	issuedStart = false;
	rt_mutex_release(&autoStart);
	rt_task_resume(&recvThread[0]); 
	rt_task_resume(&recvThread[1]);
      }
    }
      //join on receive
      rt_task_join(&recvThread[0]);
      rt_task_join(&recvThread[1]);
    return 0;
}
