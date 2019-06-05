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
#include <cstdio>
#include <stdio.h>
#include <sstream>

using namespace std;

//Global variables
int sharedMem;
int msqid1;
int msqid2;

struct my_msgbuf {
    long mtype;
    char mtext[20];
  };


/*--------------------------------------------------------------------------------------------------------*/
int getlength(int integer)
{
  int length = 1;
  int s = integer;
  if(s < 0)
  {
    s = s * (-1);
  }
  while(s > 0)
  {
    s = s / 10;
    length++;
  }
}

char* convertToString(int integer)
  {
    int length = getlength(integer);

    char * newString = (char* ) malloc (sizeof(char) * length);

    sprintf(newString,"%d",integer);
    newString[length - 1] = '\0';

    return newString;
  }

/*--------------------------------------------------------------------------------------------------------*/
//function to kill all the children
bool killEverything(int childOne, int childTwo)
{
   int childOneStatus, childTwoStatus;

      printf("Sending SIGUSR1 signals to both children\n");

      kill(childOne, SIGUSR1);
      waitpid(childOne, &childOneStatus, 0);

      //printf("Got the first child\n");

      kill(childTwo, SIGUSR1);
      waitpid(childTwo, &childTwoStatus, 0);

       printf("Child %d terminated normally with the value %d\n", childOne, WEXITSTATUS(childOneStatus));
       printf("Child %d terminated normally with the value %d\n", childTwo, WEXITSTATUS(childTwoStatus));
    }

/*--------------------------------------------------------------------------------------------------------*/
//function to free everything
void freeEverything(int sharedMem, int msqid1, int msqid2)
{
  //remove the message queues
  if ((msgctl(msqid1, IPC_RMID, 0)) == -1)
  {
        perror ("Error freeing message queues");
        //exit(-8);
  }
  if ((msgctl(msqid2, IPC_RMID, 0)) == -1)
  {
        perror ("Error freeing message queues");
        //exit(-8);
  }

  //remove the share memeory
  if ((shmctl (sharedMem, IPC_RMID, NULL)) == -1)
  {
    perror ("Error freeing shared memory");
    //  exit (-8);
  }
}

/*--------------------------------------------------------------------------------------------------------*/
//Signal Handler
void childSignalHandler(int signal)
{
    if(signal == SIGCHLD)
    {
        // printf("Reached! %d\n", signal);
        // int status;
        // int pid = waitpid(-1, &status, 0);
        //
        // if ( WIFEXITED(status) != 0 )
        //     printf("\nChild %d terminated normally with the status %d\n", pid, WEXITSTATUS(status));
        // else {
        //       printf("\nChild %d exited by signal %d\n", pid, WTERMSIG(status));
        //       freeEverything(sharedMem, msqid1, msqid2);
        //       exit(-17);
        // }
    }

    else if(signal == SIGPIPE)
    {
      perror("Pipe broke");
      exit(-1);
    }

    else if(signal == SIGINT)
    {
      freeEverything(sharedMem, msqid1, msqid2);
      exit(-1);
    }

  }


/*--------------------------------------------------------------------------------------------------------*/
//Child one
void _childOne(int mypipe1[2], int mypipe2[2], int messagequeue, int sharedmem, int &childOne) //fork the child one
{
  if((childOne = fork()) < 0) //fork
  {
      perror("error forking the first child");
      exit(-5);
  }

  if(childOne == 0) //Only entered by childOne
  {

    close(mypipe1[1]);  //?????? RETHINK about this
    close(mypipe2[0]);

    //redirect stdin and stdout
    dup2(mypipe1[0], 0);
    dup2(mypipe2[1], 1);

    //exec
    execlp("./mandelCalc", "mandelCalc", convertToString(sharedmem),convertToString(messagequeue), NULL);
    cout << "ChildOne" << endl; //should not reach here
    exit(-6); //done with the child
  }

}

/*--------------------------------------------------------------------------------------------------------*/
//Child Two
void _childTwo(int mypipe1[2], int mypipe2[2], int msqid1, int msqid2, int sharedmem, int &childTwo) //fork the child two
{

  if((childTwo = fork()) < 0) //fork
  {
      perror("error forking the first child");
      exit(-5);
  }

  if(childTwo == 0)
  {
    close(mypipe1[0]);  //Close the parent read
    close(mypipe1[1]); //close the parent write
    close(mypipe2[1]); //close the child2 write

    //redirect stdin
    dup2(mypipe2[0], 0);

    //exec
    execlp("./mandelDisp", "mandelDisp", convertToString(sharedmem), convertToString(msqid1), convertToString(msqid2), NULL);
    cout << "ChildTwo" << endl; //Should not reach here
    exit(-6); //done with the child

  }
}


/*--------------------------------------------------------------------------------------------------------*/
//While loop to ask for user input regularily
void whileLoop(int mypipe1[2], int msqid1, int msqid2, int sharedmem, int childOne, int childTwo)
{

  double xMin = 0.0, xMax = 0.0, yMin = 0.0, yMax = 0.0;
  int nRows = 0, nCols = 0, maxIters = 0;
  char check;
  FILE *file = fdopen(mypipe1[1], "w"); //close the file after
  char filename[20] = "hello.txt";
  while(true)
  {
    //fflush(stdin);

    cout << "Enter Filename: ";
    cin >> filename;
    cout << "Enter xMin: ";
    cin >> xMin;
    cout << "Enter xMax: ";
    cin >> xMax;
    cout << "Enter yMin: ";
    cin >> yMin;
    cout << "Enter yMax: ";
    cin >> yMax;
    cout << "Enter nRows: ";
    cin >> nRows;
    cout << "Enter nCols: ";
    cin >> nCols;
    cout << "Enter maxIters: ";
    cin >> maxIters;


    printf("Values: %lf %lf %lf %lf %d %d %d\n", xMin, xMax, yMin, yMax ,nRows, nCols, maxIters);

    if(xMin >= xMax || yMin >= yMax || nRows <= 0 || nCols <= 0 || maxIters <= 0) //check if valid
    {
      cout << "The numbers are invalid, try again" << endl;
      continue;
    }

    //send the details to child 1

     fprintf(file, "%lf %lf %lf %lf %d %d %d\n", xMin, xMax, yMin, yMax ,nRows, nCols, maxIters);
     fflush(file);


    //send the filename over to the message queue 2
    struct my_msgbuf newMessage;
    filename[19] = '\0';
    newMessage.mtype = 1;
    strncpy(newMessage.mtext, filename, 20);
    if((msgsnd(msqid2, &newMessage, 20 , 0)) < 0)
    {
      perror("Error sending the filename over message queue 2");
      exit(-7);
    }

    //listen for child 1

    char* child1Done;
    struct my_msgbuf rcdmessage;
    (msgrcv(msqid1, &rcdmessage, 20, 1, 0));
    child1Done = (char*)malloc(sizeof(char) *(1 + strlen(rcdmessage.mtext)));
    strcpy(child1Done, rcdmessage.mtext);


    //listen for child 2
    char* child2Done;
    struct my_msgbuf rcdmessage2;
    (msgrcv(msqid1, &rcdmessage2, 20, 1, 0));
    child2Done = (char*)malloc(sizeof(char) *(1 + strlen(rcdmessage2.mtext)));
    strcpy(child2Done, rcdmessage2.mtext);

    free(child1Done);
    free(child2Done);

    cout << "Would you like to run mandelBrot again? Press Y to run again ";
    fscanf(stdin, " %c", &check);

    if((check != 'y') && (check != 'Y'))
    {
      killEverything(childOne, childTwo);
      fclose (file);
      return;
    }
      printf("Running again\n");
      fflush(stdin);
      fflush(stdout);
      continue;

  }
}

/*--------------------------------------------------------------------------------------------------------*/
//setup the signal handlers
void setupSignals()
{
   // /Set up child signal handler
    if (signal(SIGPIPE, childSignalHandler) == SIG_ERR)
    {
        perror("Cant setup signal handler for the child");
        exit(-3);
      }

       // /Set up child signal handler
    if (signal(SIGCHLD, childSignalHandler) == SIG_ERR)
    {
        perror("Cant setup signal handler for the child");
        exit(-3);
    }

             // /Set up child signal handler
    if (signal(SIGINT, childSignalHandler) == SIG_ERR)
    {
        perror("Cant setup signal handler for the child");
        exit(-3);
    }
}


/*--------------------------------------------------------------------------------------------------------*/
//Main Function
int main()
{
    cout << "Program by Srinivas C Lingutla (slingu2), CS 361, Homework 4" << endl;

    setupSignals();


    /* Create the pipe. */
    int mypipe1[2] = {0,1}, mypipe2[2] = {0,1};

    if ((pipe (mypipe1)) < 0)
    {
        perror ("Pipe failed.\n");
        exit(-1);
      }

    if ((pipe (mypipe2)) < 0)
    {
        perror ("Pipe failed.\n");
        exit(-1);
    }

    /* Create the message queues. */
  //  int msqid1;
    if ((msqid1 = msgget(IPC_PRIVATE, IPC_CREAT | 0600 )) == -1)
    {
      perror("msgget 1 failed");
      exit(-2);
    }

  //  int msqid2;
    if ((msqid2 = msgget(IPC_PRIVATE, IPC_CREAT | 0600 )) == -1)
    {
      perror("msgget 2 failed");
      exit(-2);
    }

    /* Create the shared memory. */
  //  int sharedMem;
    if ((sharedMem = shmget(IPC_PRIVATE, 500000 , IPC_CREAT | 0600)) < 0)  //TODO size
    {
        perror("shared memory failed");
        exit(-4);
      }

    //Call off each child
    int childOne = -1, childTwo = -1;
    _childOne(mypipe1, mypipe2, msqid1, sharedMem, childOne);
    _childTwo(mypipe1, mypipe2, msqid1, msqid2, sharedMem, childTwo);

    //Close unused pipes
    close(mypipe1[0]); //close the read pipe to child one
    close(mypipe2[0]); //close the read and write pipes between the children
    close(mypipe2[1]);

    //dup2(mypipe1[1], 1);

    whileLoop(mypipe1, msqid1, msqid2, sharedMem, childOne, childTwo); //loop for user input
    freeEverything(sharedMem, msqid1, msqid2); //free message queues and shared memeory

    return 0;
}
