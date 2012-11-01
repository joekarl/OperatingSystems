#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "msg_defines.h"

void invalid_usage()
{
  printf("Invalid Usage\n");
  printf("Usage -- <id>\n");
}

int main(int argc, char ** argv)
{
  srand(time(NULL));
  
  if (argc < 2)
  {
    invalid_usage();
    return 1;
  }

  //read id from args
  int id = (int) strtol(argv[1], (char **)NULL, 10);

  //create queue
  int queue_id = msgget(id, IPC_CREAT | IPC_EXCL | 0400 | 0200);
 
  //queue already exists, delete it and recreate
  if (queue_id == -1)
  {
    queue_id = msgget(id, IPC_CREAT | 0400 | 0200);
  }

  //sacrificial message
  QueueMessage *message = malloc(sizeof(QueueMessage));
  int message_size = sizeof(QueueMessage) - sizeof(long);

  //start run loop
  int request_id = 0;
  int running = 1;
  struct timeval time;
  gettimeofday(&time, NULL);
  while (running)
  {
    usleep(100000 * (rand() % 5));
    
    message->client_id = (long)id;
    message->mType = kWait;
    message->request_id = request_id;
    gettimeofday(&time, NULL);
    msgsnd(queue_id, message, message_size, IPC_NOWAIT);
    printf("client: %d, request, request_id: %d, time: %d\n", id, request_id, (int)time.tv_sec);

    msgrcv(queue_id, message, message_size, SERVER_MID, 0);
    switch (message->mType)
    {
      case kAck:
        gettimeofday(&time, NULL);
        printf("client: %d, ack, ack_id: %d, time: %d\n", id, request_id, (int)time.tv_sec);
        break;
      case kTerminate:
        running = 0; 
        printf("Terminating client: %d\n", id);
        break;
      case kSignal:
      case kWait:
        break;
    }
    
    if (!running) 
    {
      break;
    }
   
    usleep(100000 * (rand() % 5));
    message->client_id = (long)id;
    message->mType = kSignal;
    message->request_id = request_id;
    msgsnd(queue_id, message, message_size, 0); 
    gettimeofday(&time, NULL);
    printf("client: %d, release, release_id: %d, time: %d\n", id, request_id, (int)time.tv_sec);
    request_id++;
  }

  //destroy queue
  msgctl(
      msgget(id, IPC_NOWAIT), 
      IPC_RMID,
      NULL);

  //destroy message
  free(message);

  return 0;
}
