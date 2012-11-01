#ifndef MSG_DEFINES_H
#define MSG_DEFINES_H

#define SERVER_MID 1

typedef enum {
  kSignal,
  kWait,
  kTerminate,
  kAck
} MessageType;

typedef struct
{
  long client_id;
  int request_id;
  MessageType mType;
} QueueMessage;

#endif
