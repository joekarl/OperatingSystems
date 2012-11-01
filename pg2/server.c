#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <errno.h>
#include "msg_defines.h"

int message_size = sizeof(QueueMessage) - sizeof(long);

void invalid_arguments() 
{
  printf("Invalid arguments.\n");
  printf("Usage -- <server_id> <initial semaphore count> ");
  printf("<duration> [<client_id>,....,<client_id>]\n");
}

typedef struct
{
  int request_id;
} QueuedRequest;

void send_ack(int queue_id, QueueMessage *message, int ack_id, int client_id)
{
  message->client_id = SERVER_MID;
  message->mType = kAck;
  message->request_id = ack_id;
  printf("Server acknowledging client: %d with ack: %d\n", client_id, message->request_id);
  msgsnd(queue_id, message, message_size, 0);
}

int main(int argc, char** argv) 
{
  //for for loops
  int i, j;
  
  if (argc < 4) {
    invalid_arguments();
    return 1;
  }
  
  int server_id = -1,
      semaphore = -1,
      duration = -1;

  int client_count = argc - 4;
  int client_ids[client_count];
  int client_queues[client_count];
  QueuedRequest queued_requests[client_count];
  for (i = 0; i < client_count; ++i)
  {
    client_ids[i] = -1;
    client_queues[i] = -1;
    queued_requests[i].request_id = -1;
  }

  //read in args
  server_id = (int) strtol(argv[1], (char **)NULL, 10);
  semaphore = (int) strtol(argv[2], (char **)NULL, 10);
  duration = (int) strtol(argv[3], (char **)NULL, 10);
  
  for (i = 4, j = 0; i < argc; ++i, ++j) 
  {
    client_ids[j] = (int) strtol(argv[i], (char **)NULL, 10);
    msgctl(
        msgget(client_ids[j], IPC_NOWAIT), 
        IPC_RMID,
        NULL);
  }

  //setup message queues
  for (i = 0; i < client_count; ++i)
  {
    while (client_queues[i] == -1)
    {
      client_queues[i] = msgget(client_ids[i], 0); 
    }
  }
  
  //sacrificial message
  QueueMessage *message = malloc(sizeof(QueueMessage));

  //start run loop
  int running = 1;
  struct timeval time;
  gettimeofday(&time, NULL);
  int curtime = time.tv_sec;
  int prevtime = curtime;
  while(running)
  {
    //read from queues (NO_WAIT)
    for (i = 0; i < client_count; ++i)
    {
      if (queued_requests[i].request_id != -1)
      {
        if (semaphore >= 0)
        {
          send_ack(client_queues[i], message, queued_requests[i].request_id, client_ids[i]);
          queued_requests[i].request_id = -1;  
          semaphore--;
        }
      }
      else
      {
        errno = 0;
        int rcv_flag = msgrcv(client_queues[i], message, message_size, client_ids[i], IPC_NOWAIT);
        if (rcv_flag != -1)
        {
          printf("Server received request: %d from client: %d\n", message->request_id, client_ids[i]);
          //*
          switch (message->mType)
          {
            case kSignal:
              semaphore++; 
              break;
            case kWait:
              if (semaphore >= 0)
              {
                send_ack(client_queues[i], message, message->request_id, client_ids[i]);
                semaphore--;
              } else {
                queued_requests[i].request_id = message->request_id;
              }
              break;
            case kTerminate:
            case kAck:
              break;
          }
          message->mType = kAck;
          //*/
        } 
        else
        {
        }
      }
    }

    gettimeofday(&time, NULL);
    curtime = time.tv_sec;

    //decide if running still
    if ((curtime - prevtime) > duration) {
      running = 0;
    }
  }

  //send terminate to client threads
  //and let clients destroy msg queues
  for (i = 0; i < client_count; ++i) 
  {
    message->client_id = SERVER_MID;
    message->mType = kTerminate;
    printf("Sending terminate to client: %d\n", client_ids[i]);
    msgsnd(client_queues[i], message, message_size, 0);
  }

  //cleanup temp message
  //free(message);

  return 0;
}
