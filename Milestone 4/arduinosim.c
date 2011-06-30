/* arduinosim.c 
   Similator for the arduino microcontroller as a controller in Pong
   Idris Soule
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

int usbDevice;

void sigHdl(int sig)
{
    close(usbDevice);
}

int main()
{
  struct termios options;
  const char *path = "/dev/ttyUSB0";
  
  signal(SIGINT, sigHdl);

  puts("-------------------------------");
  puts(" Arduino Microcontroller Data");
  puts("-------------------------------");

  if((usbDevice = open(path, O_RDONLY | O_NOCTTY | O_NDELAY)) < 0){
      fprintf(stderr, "Couldn't Open \"%s\"", path);
      exit(-1);
  }
  
  fcntl(usbDevice, F_SETFL, 0); //blocking if no data
  
  unsigned int nbytes = 0;
  unsigned char buffer[1];
  unsigned char data[4];
  while((nbytes = read(usbDevice, buffer, 1)) > 0){
  /*
  if(nbytes == 3){
        data[nbytes] = '\0';
        printf("=> %s\n", data);
        nbytes = 0;
      }
      data[nbytes++] = buffer[0];
  */
  putchar(buffer[0]);
  }
  return 0;
}

   

    


