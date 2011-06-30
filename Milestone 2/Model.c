#include <string.h>
//#include &ltstdlib.h>

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
    int created;
};
const  DEFAULT_HEIGHT =10;
const  LENGTH_DETA = 40;
const DEFAULT_RADIUS =20;
//at Model class
#include <stdio.h>
int resolution_x;
int resolution_y;
int resolution_w;
int resolution_h;
//Pong Class
const MAX_PONGS_PER_SIDE = 1;
const MAX_PONGS = 2;
const default_height = 467;
const default_ballSpeedX = 0;
const default_ballSpeedY = 1;
const player = 1;
const opponent =0;
const char rightTurn[1] = "R";
const char leftTurn[1] = "L";

const int pongArea1 = 10;
const int pongArea2 = 20;
const int pongArea3 = 30;
const int pongArea4 = 50;
const int pongArea5 = 60;
const int pongArea6 = 70;
const int pongArea7 = 80;

const int RlargeAngleBounce = 3;
const int RnormalAngleBounce =2;
const int RsmallAngleBounce =1;
const int noAngleBounce =0;
const int LlargeAngleBounce =-3;
const int LnormalAngleBounce =-2;
const int LsmallAngleBounce =-1;

const int directionChange =-1;
const int prepare =1;
const int Ready =2;
const int standardPongSpeed = 10;

int player_score =0;
int opponent_score =0;

int reset = 0; //false in this case
int bounceplayer = 0; //false in this case
int bounceOpponent = 0; //false in this case

int locationPointOfX;
int oppoentPointLocationX;

int gameReady =0;

int currentBallOfX;
int currentBallOfY;

int defaultBalllocationX;
int defaultBalllocationY;

int balldirectionX;
int balldirectionY;

int gameBegin = 0; //false in this case

int AIMode = 0; //false in this case
int PlayerMode = 0; //false in this case

struct pongWidget pongs[2];
struct BallWidget ball;
int nPongs;
int pongLength;

  char right[] = "R";
    char left[] = "L";


void pongInitial(){
    pongLength = MAX_PONGS* LENGTH_DETA;
    ball.created = 0;
}

void bouncingCheck (int x,int y, int radius, int directionX,int directionY){
    if(nPongs >= MAX_PONGS){
        if(x + directionX < resolution_x){
           // changeBallDirectionHorizontal();
        }
        else if(x +directionX + radius > resolution_w){
            //changeBallDirectionHorizontal();
        }
        /*
         *Pong checking, ift he ball hit the bar then it bounce
         */
         if (y+ directionY - radius +pongs[opponent].height >pongs[opponent].leftCorner.point_y){
             if(x+directionX + radius > pongs[opponent].leftCorner.point_x &&
                x+directionX + radius < pongs[opponent].leftCorner.point_x+pongLength+radius){
                    bounceOpponent = 1; //set it to true
                    //changeBallDirectionVertical();
                }
             else{
                 player_score++;
                 //call the prox to update score-> player_score and opponent_score;
                 reset = 1;
             }
         }
        if (y+ directionY + radius +pongs[player].height >pongs[player].leftCorner.point_y){
             if(x+directionX + radius > pongs[player].leftCorner.point_x &&
                x+directionX + radius < pongs[player].leftCorner.point_x+pongLength+radius){
                    bounceplayer = 1; //set it to true
                    //changeBallDirectionVertical();
                }
             else{
                 opponent_score++;
                 //call the prox to update score-> player_score and opponent_score;
                 reset = 1;
             }
         }
    }
}

void bounceDirection(int pongIdentifier,int radius){
    int tempBallX = currentBallOfX + balldirectionX;
    int area1 = pongs[pongIdentifier].leftCorner.point_x + pongArea1;
    int area2 = pongs[pongIdentifier].leftCorner.point_x + pongArea2;
    int area3 = pongs[pongIdentifier].leftCorner.point_x + pongArea3;
    int area4 = pongs[pongIdentifier].leftCorner.point_x + pongArea4;
    int area5 = pongs[pongIdentifier].leftCorner.point_x + pongArea5;
    int area6 = pongs[pongIdentifier].leftCorner.point_x + pongArea6;
    int area7 = pongs[pongIdentifier].leftCorner.point_x + pongArea7;


    if(currentBallOfX + radius >= pongs[pongIdentifier].leftCorner.point_x && tempBallX < area1){
        balldirectionX = LlargeAngleBounce;
    }
    else if (tempBallX >= area1 && tempBallX < area2){
        balldirectionX = LnormalAngleBounce;
    }
    else if (tempBallX >= area2 && tempBallX < area3){
        balldirectionX = LsmallAngleBounce;
    }
    else if (tempBallX >= area1 && tempBallX < area4){
        balldirectionX = noAngleBounce;
    }
    else if (tempBallX >= area1 && tempBallX < area5){
        balldirectionX = RsmallAngleBounce;
    }
    else if (tempBallX >= area1 && tempBallX < area6){
        balldirectionX = RnormalAngleBounce;
    }
     else if (tempBallX >= area1 && tempBallX < area7){
        balldirectionX = RlargeAngleBounce;
    }
}
// Changing the ball direction by multipy -1 with the current direction
void changeBallDirectionHorizontal(){
    balldirectionX = balldirectionX * directionChange;
}
// Changing the ball direction by multipy -1 with the current direction
void changeBallDirectionVertical(){
    balldirectionY = balldirectionY * directionChange;
}

void ballmove(){
    if(nPongs >= MAX_PONGS){
            currentBallOfX = ball.centre.point_x;
            currentBallOfY = ball.centre.point_y;
            bouncingCheck(currentBallOfX,currentBallOfY, ball.radius, balldirectionX,balldirectionY);
            currentBallOfX = currentBallOfX + balldirectionX;
            currentBallOfY = currentBallOfY + balldirectionY;
            ball.centre.point_x = currentBallOfX;
            ball.centre.point_y = currentBallOfY;
            //send msg to prox  (currentBall OfX,currentBallOfY,Radius)
    }
}

void setResolution(int x, int y, int w, int h){
   resolution_x = x;
   resolution_y = y;
   resolution_w = w;
   resolution_h = h;
}

void widgetNotification(struct pongWidget p_widget){
    pongs[nPongs++] = p_widget;
}

//original function in java call reset
void resetBall(){
    if(ball.created == 1){
        ball.centre.point_x =defaultBalllocationX;
        ball.centre.point_y = defaultBalllocationY;
        gameReady = 1;
        gameBegin = 0;
        balldirectionX = default_ballSpeedX;
        balldirectionY = default_ballSpeedY;
         //send msg to prox for update score( player score, opponent score)
    }
}
void resetScore(){

        if(nPongs>= MAX_PONGS){
            player_score = 0;
            opponent_score = 0;
            //send msg update for the score(playerscore, opponent_score)
        }

}
void start (){
    if(AIMode == 1 || PlayerMode ==1){
        if(gameReady<prepare){
            addBall();
            //rt_tast_create...
            //rt_tast_start
            gameReady++;
        }
        else if (gameReady == prepare){
            gameReady++;
        }
        else if (gameReady ==Ready){
            gameBegin = 1;
        }
    }
}

void activePlayerMode(int mode){
    PlayerMode = mode;
}
void activeAiMode(int mode){
    AIMode = mode;
}

void addBall(){
    if(ball.created ==0){
        ball.centre.point_x = (resolution_w-resolution_x)/2;
        ball.centre.point_y = (resolution_h-resolution_y)/2;
        defaultBalllocationX = (resolution_w-resolution_x)/2;
        defaultBalllocationY =  (resolution_h-resolution_y)/2;
        // pPanel notify for created pPanel
        balldirectionX = default_ballSpeedX;
        balldirectionY = default_ballSpeedY;
        // sPanel notify for update ball (playerscore,opponent score
    }
    //pPanel paintballNotify (ball.centre.point_x, ball.centre.point_y, ball.radius)
}

void addPong(){
    if (nPongs < MAX_PONGS){
        pongs[nPongs].leftCorner.point_x = ((resolution_w-resolution_x)/2 - (pongLength/2));
        pongs[nPongs].leftCorner.point_y = resolution_y;
        oppoentPointLocationX = ((resolution_w-resolution_x)/2 - (pongLength/2));
        //pPanel.pongCreated();
        //pPanel.paintOpponentPongNotify (pongs[nPongs].leftCorner.point_x, pongs[nPongs].leftCorner.point_y,pongLength,  pongs[nPongs].height);
        pongs[nPongs].length = pongLength;
        nPongs++;
        pongs[nPongs].leftCorner.point_x =((resolution_w-resolution_x)/2 - (pongLength/2));
        pongs[nPongs].leftCorner.point_y = default_height;
        locationPointOfX = ((resolution_w-resolution_x)/2 - (pongLength/2));

        //pPanel.pongCreated();
        //pPanel.paintPongNotify (pongs[nPongs].leftCorner.point_x, pongs[nPongs].leftCorner.point_y,pongLength,  pongs[nPongs].height);\
        pongs[nPongs].length = pongLength;
        nPongs++;
    }
}
int illegal_move_check (int check_location, int check_out_of_bound){
     if(check_location +pongLength +check_out_of_bound + standardPongSpeed > resolution_w){
         return 0;
     }
     else if(check_location +check_out_of_bound < 0){
         return 0;
     }
     return 1;
}
void move(char str[]){


    if(strcmp(str,right) == 0){
          if (illegal_move_check(locationPointOfX, standardPongSpeed) == 1){
              locationPointOfX = locationPointOfX + standardPongSpeed;
              pongs[player].leftCorner.point_x = locationPointOfX;
              pongs[player].leftCorner.point_y = default_height;
          }
      }
    else if (strcmp(str,left) == 0){
        if (illegal_move_check(locationPointOfX, standardPongSpeed) == 1){
              locationPointOfX = locationPointOfX - standardPongSpeed;
              pongs[player].leftCorner.point_x = locationPointOfX;
              pongs[player].leftCorner.point_y = default_height;
          }
       }
       //pPanel.paintPongNotify (pongs[player].leftCorner.point_x, pongs[player].leftCorner.point_y,pongLength,  pongs[player].height);\

}

void opponentMovement(char strOpponent[]){
    if(strcmp(strOpponent,right) == 0){
          if (illegal_move_check(oppoentPointLocationX, standardPongSpeed) == 1){
              oppoentPointLocationX = oppoentPointLocationX + standardPongSpeed;
              pongs[opponent].leftCorner.point_x = oppoentPointLocationX;
              pongs[opponent].leftCorner.point_y = resolution_y;
          }
    }
    else if (strcmp(strOpponent,left) == 0){
        if (illegal_move_check(oppoentPointLocationX, standardPongSpeed) == 1){
              oppoentPointLocationX = oppoentPointLocationX - standardPongSpeed;
              pongs[opponent].leftCorner.point_x = oppoentPointLocationX;
              pongs[opponent].leftCorner.point_y = resolution_y;
          }
    }
   //pPanel.paintOpponentPongNotify (pongs[opponent].leftCorner.point_x, pongs[opponent].leftCorner.point_y,pongLength,  pongs[opponent].height);
}

char AImoveGenerate(){
   int n;
   n=random(2);  /* n is random number in range of 0 - 99 */
   if(n == 0){
       return right;
   }
   return left;

}
// A work Thread for run ball
void run(void *cookie){
    while (1){
        if(ball.created == 1){
            if (gameReady == Ready){
                ballmove();
            }
        }
        if(AIMode == 1){
            opponentMovement(AImoveGenerate);
        }
        if(reset ==1){
            resetBall();
            reset = 0;
        }
        if(bounceplayer == 1){
            bounceDirection(player,ball.radius);
            bounceplayer = 0;
        }
        if(bounceOpponent == 1){
            bounceDirection(opponent,ball.radius);
            bounceOpponent = 0;
        }
        //Thread...sleep
    }
}

int main()
{
    pongInitial();
    //rt_task_create(.......
    return 0;
}
