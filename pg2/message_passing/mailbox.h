#ifndef MAILBOX_H
#define MAILBOX_H

#include <pthread.h>

typedef struct ThreadNode
{
  pthread_t thread;
  int *semaphore;
  struct ThreadNode* next;
} ThreadNode;

typedef struct
{
  long mId;
} Message;

typedef struct MessageNode
{
  Message* message;
  struct MessageNode* next;
} MessageNode;

typedef struct
{
  int max_entries;
  int full_entries;
  int empty_entries;
  ThreadNode *send_queue;
  ThreadNode *receive_queue;
  MessageNode *message_queue;
} Mailbox;


#endif
