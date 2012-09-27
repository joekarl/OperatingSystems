#ifndef MessageQueueFunctions_H
#define MessageQueueFunctions_H

#include <exception>
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <cstddef>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include "defines.h"
using namespace std;

//template message struct
template <class T>
struct message_queue_client_struct
{
  long mtype;
  T message;
};

//template method to determine the size of a message struct
template <class T>
inline int sizeof_message_queue_client_struct()
{
  static int message_size = sizeof(struct message_queue_client_struct<T>) - sizeof(long);
  return message_size;
}

//custom exception to throw because msgsnd/msgrcv/msgget are c methods that can't use exceptions....
class QueueConnectionException 
{
  const char* what() const throw()
  {
    return "Queue not connected....";
  }

};

//Class to handle communication with a message queue
//Must call attach to queue before trying to use send or read message
template <class T>
class MessageQueueClient
{
  //queue key
  int _message_queue_key;
  //internal queue id
  int _message_queue_id;

  public:
    //constructor
    MessageQueueClient(int message_queue_key)
      : _message_queue_key(message_queue_key), _message_queue_id(-1)
    {};

    //Put a message in this message queue with the message type of fromAgentId
    //So that the receiver can get messages from specific agents
    void sendMessage(const T & message, long fromAgentId) throw(QueueConnectionException)
    {
      if (_message_queue_id == -1)
      {
        attachToQueue(-1);
      }
      message_queue_client_struct<T> msg = {fromAgentId, message};
      int snd_return = -1;
      int flags = 0;
      snd_return = msgsnd(_message_queue_id, &msg, sizeof_message_queue_client_struct<T>(), flags);
      
      //cout << "finished sending message to queue " << _message_queue_key << " for agent " << fromAgentId << endl;      
      //If we faild to send, throw exception
      if (snd_return == -1) 
      {
        throw QueueConnectionException();
      }
    };

    //Read a message from this queue with message type fromAgentId
    //So that we can specify which agent we're wanting to get a message from
    //The noWait bool can be used to specify whether or not you want to block on this read
    T readMessage(long fromAgentId, bool noWait) throw(QueueConnectionException)
    {
      if (_message_queue_id == -1)
      {
        attachToQueue(-1);
      }
      message_queue_client_struct<T> msg;
      int rcv_return = -1;
      int flags = 0;
      if (noWait)
      {
        flags = flags || IPC_NOWAIT;
      }
      //cout << "waiting for message in queue " << _message_queue_key << " from agent " << fromAgentId << endl;

      rcv_return = msgrcv(_message_queue_id, &msg, sizeof_message_queue_client_struct<T>(), fromAgentId, flags);
      
      if (rcv_return == -1) 
      {
        throw QueueConnectionException();
      }
      
      return msg.message;
    };
    
    //blocking call to attach client to queue
    //if connection_timeout is > 0 then the call will try to connect to queue for connection_timeout seconds
    //if connection_timeout == 0 then the call will try only once
    //if connection_timeout < 0 then the call will block until the queue appears and WILL NOT timeout
    //will return -1 for failure
    void attachToQueue(int connection_timeout)
    {
      int queue_id = -1;
      struct timeval time;
      gettimeofday(&time, NULL);
      int curtime = time.tv_sec;
      int prevtime = curtime;
      bool ipcWait = connection_timeout < 0;
      if (!ipcWait)
      {
        bool do_once = connection_timeout == 0;
        while (queue_id == -1 && (curtime - prevtime) < connection_timeout)
        {
          gettimeofday(&time, NULL);
          curtime = time.tv_sec;
          queue_id = msgget(_message_queue_key, IPC_NOWAIT);
          if (do_once) 
          {
            break;
          }
        }
      } 
      else
      {
        //cout << "attaching to queue with key " << _message_queue_key << " in blocking mode " << endl;
        while (queue_id == -1)
        {
          queue_id = msgget(_message_queue_key, 0);
          usleep(SLEEP_TIME);
        }
      }

      //cout << "atteched to queue with key " << _message_queue_key << " an queue id " << queue_id << endl;

      _message_queue_id = queue_id;
    };
    
};


#endif
