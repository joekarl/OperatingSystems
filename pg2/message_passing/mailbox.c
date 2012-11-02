#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include "mailbox.h"

void invalid_usage()
{
  printf("Invalid Usage\n");
  printf("Usage -- <size of mailbox> <duration in seconds> <#of senders> <#of receivers>\n");
}

//shared variables....
int running = 1;
Mailbox *mailbox = 0;
pthread_mutex_t mutex;
int message_id;

void * sender_thread();
void * receiver_thread();

int main (int argc, char ** argv)
{
  //for loops
  int i = 0;

  if (argc < 5)
  {
    invalid_usage();
    return 1;
  }

  //read in args
  int mailbox_size = -1,
      duration = -1,
      sender_count = -1,
      receiver_count = -1;


  mailbox_size = (int) strtol(argv[1], (char **)NULL, 10);
  duration = (int) strtol(argv[2], (char **)NULL, 10);
  sender_count = (int) strtol(argv[3], (char **)NULL, 10);
  receiver_count = (int) strtol(argv[4], (char **)NULL, 10);

  //init Mailbox
  mailbox = malloc(sizeof(Mailbox));
  mailbox->empty_entries = mailbox_size;

  //start threads....
  pthread_t sender_threads[sender_count];
  pthread_t receiver_threads[receiver_count]; 
  int max_threads = sender_count > receiver_count ? sender_count : receiver_count;
  for (i = 0; i < max_threads; ++i)
  {
    int max_retries = 10;
    int retry_count = -1;
    if (i < sender_count)
    {
      retry_count = -1;
      int err = 11;
      while (err == 11 && retry_count < max_retries)
      {
        //printf("Creating send thread: %d\n", i);
        err = pthread_create(&(sender_threads[i]), NULL, sender_thread, NULL);
        retry_count++;
      }
      if (err != 0 || retry_count == max_retries) {
        printf("Error creating threads.... exiting\n");
        printf("Error was %d\n", err);
        if (err == 11)
        {
          printf("Too many threads were created. Try using a smaller number of threads\n");
        }
        return err;
      }
    }
    if (i < receiver_count)
    {
      retry_count = -1;
      int err = 11; 
      while (err == 11 && retry_count < max_retries)
      {
        //printf("Creating receive thread: %d\n", i);
        err = pthread_create(&(receiver_threads[i]), NULL, receiver_thread, NULL);
        retry_count++;
      }
      if (err != 0 || retry_count == max_retries) {
        printf("Error creating threads.... exiting\n");
        printf("Error was %d\n", err);
        if (err == 11)
        {
          printf("Too many threads were created. Try using a smaller number of threads\n");
        }
        return err;
      }
    }
  }

  struct timeval time;
  gettimeofday(&time, NULL);
  int curtime = time.tv_sec;
  int prevtime = curtime; 
  while (running)
  {

    gettimeofday(&time, NULL);
    curtime = time.tv_sec;

    usleep(10);

    //decide if running still
    if ((curtime - prevtime) > duration) {
      running = 0;
    }
  }

  //delete mailbox
  if (mailbox->send_queue)
  {
    ThreadNode *list = mailbox->send_queue;
    while(list && list->next)
    {
      ThreadNode *del = list;
      list = list->next;
      pthread_cancel(del->thread); 
      free(del);
    }
  }
  if (mailbox->receive_queue)
  {
    ThreadNode *list = mailbox->receive_queue;
    while(list && list->next)
    {
      ThreadNode *del = list;
      list = list->next;
      pthread_cancel(del->thread); 
      free(del);
    }
  }
  if (mailbox->message_queue && mailbox->full_entries > 0)
  {
    MessageNode *list = mailbox->message_queue;
    while(list && list->next)
    {
      MessageNode *del = list;
      list = list->next;
      if (del && del->message)
      {
        free(del->message);
        free(del);
      }
    }
  }
  free(mailbox); 

  for(i = 0; i < sender_count; ++i)
  {
    if (sender_threads[i])
    {
      pthread_cancel(sender_threads[i]);
    }
  } 

  for(i = 0; i < receiver_count; ++i)
  {
    if (receiver_threads[i])
    {
      pthread_cancel(receiver_threads[i]);
    }
  } 
  return 0;
}


void * sender_thread()
{
  pthread_mutex_lock(&mutex); 
  //printf("locked by sender: %d\n", (int)pthread_self());
  int blocked = 0;
  while (mailbox->empty_entries == 0)
  {
    //printf("Mailbox full, add to send queue\n");
    ThreadNode *tNode = malloc(sizeof(ThreadNode));
    tNode->thread = pthread_self();
    int semaphore = 1;
    tNode->semaphore = &semaphore;
    tNode->next = mailbox->send_queue;
    mailbox->send_queue = tNode;
    //printf("unlocked by sender: %d\n", (int)pthread_self());
    pthread_mutex_unlock(&mutex); 
    blocked = 1;
    while(semaphore == 1)
    {
      usleep(10);
    }
    pthread_mutex_lock(&mutex);
    //printf("locked by sender: %d\n", (int)pthread_self());
  }

  //printf("create new message\n");
  MessageNode *mNode = malloc(sizeof(MessageNode));
  Message *m = malloc(sizeof(Message));
  m->mId = message_id++;
  mNode->message = m;
  mNode->next = NULL;
  MessageNode *list = mailbox->message_queue;
  if (list)
  {
    //printf("mailbox not empty, adding to message queue\n");
    while(list->next != NULL)
    {
      list = list->next;
    }
    list->next = mNode;
  } 
  else
  {
    //printf("mailbox empty, setting to message queue\n");
    mailbox->message_queue = mNode;
  }

  //printf("message successfully added\n");
  mailbox->empty_entries--;
  mailbox->full_entries++;

  int rcv_queue_size = 0;
  ThreadNode *tNode = mailbox->receive_queue;
  while(tNode)
  {
    tNode = tNode->next;
    rcv_queue_size++;
  }
  int send_queue_size = 0;
  tNode = mailbox->send_queue;
  while(tNode)
  {
    tNode = tNode->next;
    send_queue_size++;
  }

  printf("Sending from thread: %d, sender, mId: %ld, %s, receive_queue_size: %d, send_queue_size: %d, messages_in_queue: %d\n", 
      (int)pthread_self(), m->mId, blocked ? "Blocked" : "Not Blocked", 
      rcv_queue_size, send_queue_size, mailbox->full_entries);

  if (mailbox->receive_queue)
  {
    ThreadNode *tNode = mailbox->receive_queue;
    mailbox->receive_queue = tNode->next;
    (*(tNode->semaphore)) = 0;
    //printf("Awaking receive node....\n");
    free(tNode);
  }

  //printf("unlocked by sender: %d\n", (int)pthread_self());
  //printf("thread exiting: %d\n", (int)pthread_self());
  pthread_mutex_unlock(&mutex); 
  pthread_exit(NULL);
}

void * receiver_thread()
{
  pthread_mutex_lock(&mutex); 
  //printf("locked by receiver: %d\n", (int)pthread_self());
  int blocked = 0;
  while (mailbox->full_entries == 0) //queue empty
  {
    //printf("Mailbox empty, add to receive queue\n");
    ThreadNode *tNode = malloc(sizeof(ThreadNode));
    tNode->thread = pthread_self();
    int semaphore = 1;
    tNode->semaphore = &semaphore;
    tNode->next = mailbox->receive_queue;
    mailbox->receive_queue = tNode;
    //printf("unlocked by receiver: %d\n", (int)pthread_self());
    pthread_mutex_unlock(&mutex); 
    blocked = 1;
    while(semaphore == 1)
    {
      usleep(10);
    }
    pthread_mutex_lock(&mutex);
    //printf("locked by receiver: %d\n", (int)pthread_self());
  }

  MessageNode *next = mailbox->message_queue->next;

  int mId = mailbox->message_queue->message->mId;
  //printf("Freeing message....\n");
  free(mailbox->message_queue->message);
  free(mailbox->message_queue);
  mailbox->message_queue = next;

  mailbox->empty_entries++;
  mailbox->full_entries--;

  int rcv_queue_size = 0;
  ThreadNode *tNode = mailbox->receive_queue;
  while(tNode)
  {
    tNode = tNode->next;
    rcv_queue_size++;
  }
  int send_queue_size = 0;
  tNode = mailbox->send_queue;
  while(tNode)
  {
    tNode = tNode->next;
    send_queue_size++;
  }

  printf("Receiving from thread: %d, receiver, mId: %d, %s, receive_queue_size: %d, send_queue_size: %d, messages_in_queue: %d\n", 
      (int)pthread_self(), mId, 
      blocked ? "Blocked" : "NotBlocked", 
      rcv_queue_size, send_queue_size, mailbox->full_entries);

  if (mailbox->send_queue)
  {
    ThreadNode *tNode = mailbox->send_queue;
    mailbox->send_queue = tNode->next;
    (*(tNode->semaphore)) = 0;
    //printf("Awaken send node....\n");
    free(tNode);
  }

  //printf("unlocked by receiver: %d\n", (int)pthread_self());
  //printf("thread exiting: %d\n", (int)pthread_self());
  pthread_mutex_unlock(&mutex); 
  pthread_exit(NULL);
}





