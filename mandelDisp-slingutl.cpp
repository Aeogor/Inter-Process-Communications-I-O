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
#include <string>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sys/resource.h>
#include <stdio.h>
#include <cstdio>
#include <cstring>


using namespace std;

struct my_msgbuf {
    long mtype;
    char mtext[20];
};

int numberOfImagesCalculated = 0;\
void * data;


/*--------------------------------------------------------------------------------------------------------*/
void removeSharedMem()
{
    if (shmdt(data) == -1)
      {
        perror("shmdt");
        exit(1);
    }
}

//sig user handler
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
//Main function
int main(int argc, char** argv)
{
  int sharedmem = 0, messagequeue1 = 0, messagequeue2 = 0;
  int nColors = 15;
  char colors[15] = { '.','-','~',':','+', '*','%','O','8','&','?','$','@','#','X' };
  int fileOpened = true;

 //setup the SIGUSR1 handler
  if (signal(SIGUSR1, signalHanglerSIGUSR1) == SIG_ERR)
  {
      perror("Cant setup signal handler for the SIGUSER1");
      exit(-3);
  }

  if(argc > 2) //get the arguments
  {
    sharedmem = atoi(argv[1]);
    messagequeue1 = atoi(argv[2]);
    messagequeue2 = atoi(argv[3]);
  }



  data = shmat(sharedmem, 0, 0); //shared memory access pointer

  double xMin = 0.0, xMax = 30.0, yMin = 0.0, yMax = 30.0;
  int nRows = 30, nCols = 30, maxIters = 30;

  while(true)
  {
    //read values from stdin
     fscanf(stdin, " %lf %lf %lf %lf %d %d %d", &xMin, &xMax, &yMin, &yMax, &nRows, &nCols, &maxIters);

    //read filename
    char* filename;
    struct my_msgbuf rcdmessage;
    (msgrcv(messagequeue2, &rcdmessage, 20, 1, 0));
    filename = (char*)malloc(sizeof(char) *(1 + strlen(rcdmessage.mtext)));
    strcpy(filename, rcdmessage.mtext);


    //create a file with the filename
    ofstream myfile;
    myfile.open(filename);

    for(int r = nRows - 1; r >= 0; r--)
    {
      for(int c = 0; c < nCols ; c++)
      {
        int n = *( (int*)data + r * nCols + c );
        if( n < 0 ) cout << " ";
        else cout << colors[ n % nColors ];

        if(myfile.is_open()) myfile << n << " ";
      }
      printf("\n");
       if(myfile.is_open()) myfile << endl;
    }

    myfile.close();


    //send the filename over to the message queue 2
    struct my_msgbuf newMessage;
    char done[20] = "Done";
    done[19] = '\0';
    newMessage.mtype = 1;
    strncpy(newMessage.mtext, done, 20);
    if((msgsnd(messagequeue1, &newMessage, 20 , 0)) < 0)
    {
      perror("Error sending the filename over message queue 2");
      exit(-7);
    }

    //printf("Send Done - MandelDisp\n");
    numberOfImagesCalculated++;
  }
  return 0;
}
