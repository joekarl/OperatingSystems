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
  for (i = 0; i < sender_count; ++i)
  {
    pthread_create(&(sender_threads[i]), NULL, sender_thread, NULL);
  }

  pthread_t receiver_threads[receiver_count]; 
  for (i = 0; i < receiver_count; ++i)
  {
    pthread_create(&(receiver_threads[i]), NULL, receiver_thread, NULL);
  }

  struct timeval time;
  gettimeofday(&time, NULL);
  int curtime = time.tv_sec;
  int prevtime = curtime; 
  while (running)
  {

    gettimeofday(&time, NULL);
    curtime = time.tv_sec;

    //decide if running still
    if ((curtime - prevtime) > duration) {
      running = 0;
    }
  }
  
  //delete mailbox
  if (mailbox->send_queue)
  {
    ThreadList *list = mailbox->send_queue;
    while(list->next)
    {
      ThreadList *del = list;
      list = list->next;
      pthread_cancel(del->thread); 
      free(del);
    }
  }
  if (mailbox->receive_queue)
  {
    ThreadList *list = mailbox->receive_queue;
    while(list->next)
    {
      ThreadList *del = list;
      list = list->next;
      pthread_cancel(del->thread); 
      free(del);
    }
  }
   if (mailbox->message_queue)
  {
    MessageList *list = mailbox->message_queue;
    while(list->next)
    {
      MessageList *del = list;
      list = list->next;
      free(del->message);
      free(del);
    }
  }
  free(mailbox); 

  for(i = 0; i < sender_count; ++i)
  {
    pthread_cancel(sender_threads[i]);
  } 

  for(i = 0; i < receiver_count; ++i)
  {
    pthread_cancel(receiver_threads[i]);
  } 

  return 0;
}


void * sender_thread()
{
  //int semaphore = 0;
  pthread_mutex_lock(&mutex); 
  if (mailbox->full_entries != 0 && mailbox->empty_entries != 0)
  {
    MessageList *mNode = malloc(sizeof(MessageList));
    Message *m = malloc(sizeof(Message));
    m->mId = message_id++;
    mNode->message = m;
    MessageList *list = mailbox->message_queue;
    while(list->next)
    {
      list = list->next;
    }
    list->next = mNode;
    printf("Sending from thread: %d, sender, mId: %d, notblock, receive_queue_size: , send_queue_size: \n", (int)pthread_self(), message_id);
  }
  pthread_mutex_unlock(&mutex); 
  pthread_exit(NULL);
}

void * receiver_thread()
{
  //int semaphore = 0;
  pthread_mutex_lock(&mutex); 
  if (mailbox->full_entries != 0 && mailbox->empty_entries != 0)
  {
    MessageList *list = mailbox->message_queue->next;
    int mId = mailbox->message_queue->message->mId;
    free(mailbox->message_queue->message);
    free(mailbox->message_queue);
    mailbox->message_queue = list;
    printf("Receiving from thread: %d, receiver, mId: %d, notblock, receive_queue_size: , send_queue_size: \n", (int)pthread_self(), mId);
  }
  if (mailbox->full_entries == 0) //queue empty
  {
    pthread_mutex_unlock(&mutex); 
    ThreadList *tNode = malloc(sizeof(ThreadList));
    tNode->thread = pthread_self();
    ThreadList *list = mailbox->receive_queue;
    if (list) {
      while(list->next)
      {
        list = list->next;
      }
      list->next = tNode;
    }
    else
    {
      mailbox->receive_queue = tNode;
    }
    
  }
  pthread_mutex_unlock(&mutex); 
  pthread_exit(NULL);
}





