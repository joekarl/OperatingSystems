class MessageQueue {
  int _queueKey;
  int _messageQueueId;

  public:
    //Will initialize a posix message queue
    MessageQueue(int queueKey);
    //Will release the initialized posix message queue
    ~MessageQueue();
    int getMessageQueueId() {return _messageQueueId;};
  
  private:
    
  
};
