#ifndef MAILBOX_H
#define MAILBOX_H

#include <pthread.h>

typedef struct ThreadList
{
  pthread_t thread;
  struct ThreadList* next;
} ThreadList;

typedef struct
{
  long mId;
} Message;

typedef struct MessageList
{
  Message* message;
  struct MessageList* next;
} MessageList;

typedef struct
{
  int full_entries;
  int empty_entries;
  ThreadList *send_queue;
  ThreadList *receive_queue;
  MessageList *message_queue;
} Mailbox;


#endif
