#include "MessageQueue.h"
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h> 

using namespace std;

MessageQueue::MessageQueue(int queueKey):_queueKey(queueKey) {
  //Try to create the queue in exclusive mode....
  _messageQueueId = msgget(_queueKey, IPC_CREAT | IPC_EXCL | 0400 | 0200);
  if (_messageQueueId == -1)
  {
    //cout << "Destroying previously created queue with id " << _queueKey << endl;
    //Failed to create queue in exclusive mode so delete the original queue and try again....
    msgctl(
      msgget(_queueKey, IPC_NOWAIT), 
      IPC_RMID,
      NULL);
    _messageQueueId = msgget(_queueKey, IPC_CREAT | 0400 | 0200);
  }
  
  //cout << "Created message queue with key " << queueKey << " and id " << _messageQueueId << endl;  
}

MessageQueue::~MessageQueue() {
  msgctl(_messageQueueId, IPC_RMID, NULL);
  //cout << "Destroyed message queue with key " << _queueKey << " and id " << _messageQueueId << endl;  
}



