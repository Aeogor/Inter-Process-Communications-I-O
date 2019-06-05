//Srinivas Lingutla
//655115444
//slingu2


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <stdint.h>
#include <sys/resource.h>
#include <stdio.h>
#include <cstdio>


using namespace std;

int numberOfImagesCalculated = 0;
void * data;

struct my_msgbuf {
    long mtype;
    char mtext[20];
};

/*--------------------------------------------------------------------------------------------------------*/

void removeSharedMem()
{
    if (shmdt(data) == -1)
      {
        perror("shmdt");
        exit(1);
    }
}

//Siguser 1 handler
void signalHanglerSIGUSR1(int signal)
{
  if(signal == SIGUSR1)
  {
    perror("SIGUSR1 caught");
    removeSharedMem();
    exit(numberOfImagesCalculated);
  }
}



/*--------------------------------------------------------------------------------------------------------*/
//main function
int main(int argc, char** argv)
{
  //setup the SIGUSR1 handler
  if (signal(SIGUSR1, signalHanglerSIGUSR1) == SIG_ERR)
  {
      perror("Cant setup signal handler for the SIGUSER1");
      exit(-3);
  }

  int sharedmem = 0, messagequeue = 0;

  if(argc > 1) //get the arguments
  {
    sharedmem = atoi(argv[1]);
    messagequeue = atoi(argv[2]);
  }

  data = shmat(sharedmem, 0, 0);

  double xMin = 0.0, xMax = 30.0, yMin = 0.0, yMax = 30.0;
  int nRows = 30, nCols = 30, maxIters = 30;

  while(true)
  {
    fscanf(stdin, " %lf %lf %lf %lf %d %d %d", &xMin, &xMax, &yMin, &yMax, &nRows, &nCols, &maxIters);

    //MandelBrot algorithm
    int n;
    double deltaX = ( xMax - xMin ) / ( nCols - 1 );
    double deltaY = ( yMax - yMin ) / ( nRows - 1 );

    for(int r = 0; r < nRows; r++)
    {
      double Cy = yMin + r * deltaY;
      for(int c = 0; c< nCols ; c++)
      {
        double Cx = xMin + c * deltaX;
        double Zx = 0.0;
        double Zy = 0.0;

        for( n = 0; n < maxIters; n++ ) {
          if( (Zx * Zx + Zy * Zy) >= 4.0 )
              break;
          double Zx_next = Zx * Zx - Zy * Zy + Cx;
          double Zy_next = 2.0 * Zx * Zy + Cy;

          Zx = Zx_next;
          Zy = Zy_next;

        }

        if(n >= maxIters)  (*( (int*)data + r * nCols + c )) = -1;
        else (*( (int*)data + r * nCols + c )) = n;

      }
    }



  //write to stdout
    fprintf(stdout, "%lf %lf %lf %lf %d %d %d\n", xMin, xMax, yMin, yMax ,nRows, nCols, maxIters);
    fflush(stdout);

  struct my_msgbuf newMessage;
  char filename[20] = "Done";
  filename[19] = '\0';
  newMessage.mtype = 1;
  strncpy(newMessage.mtext, filename, 20);
  if((msgsnd(messagequeue, &newMessage, 20 , 0)) < 0)
  {
    perror("Error sending the filename over message queue 2");
    exit(-7);
  }

  numberOfImagesCalculated++;

  }

  return 0;
}
