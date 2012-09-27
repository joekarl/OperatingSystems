#include "MessageQueue.h"
#include "MessageQueueClient.h"
#include "defines.h"
#include <unistd.h>
#include <iostream>
#include <limits.h>
#include <signal.h>
#include <cmath>
#include <errno.h>
using namespace std;

#define EXPECTED_ARGS_NUM 4
#define TEMP_THRESHOLD 0.00247875217 //1.0e^-6

MessageQueue* msgQ = NULL; 

enum AgentStatus {INITIALIZED, STABILIZED };

typedef struct 
{
  AgentStatus status;
  float temperature;
} StatusMessage;

//delete global message queue
void deleteMsgQ() 
{
  delete msgQ;
}

//release our message queue and exit
void catchTerminateSignal(int param)
{
  cout << "Terminating with signal " << param << endl;
  deleteMsgQ();
  exit(1);
}

void registerSignalHandlers()
{
  //signals to be caught
  int signalCount = 5;
  int registeredSignals[signalCount];
  registeredSignals[0] = SIGINT;
  registeredSignals[1] = SIGABRT;
  registeredSignals[2] = SIGQUIT;
  registeredSignals[3] = SIGHUP;
  registeredSignals[4] = SIGTERM;

  int i = 0;
  while (i < signalCount)
  {
    //call the catch terminate method to handle cleanup 
    signal(registeredSignals[i], catchTerminateSignal);
    i++;
  }
  
}

void print_usage()
{
  cout << "Usage -- agent <initial_temperature> <id> <n1Id> <n2Id>" << endl;
}

int main(int argc, char ** argv) {
  //handle user input
  if (argc != EXPECTED_ARGS_NUM + 1) 
  {
    print_usage();
    return 1;
  }

  int initial_temperature, id, n1Id, n2Id;
  
  int* arg_order[EXPECTED_ARGS_NUM] = {&initial_temperature, 
                       &id, 
                       &n1Id, 
                       &n2Id};
  char* arg_tail;

  int i = 0;
  while(i < EXPECTED_ARGS_NUM)
  {
    (*(arg_order[i])) = strtol(argv[i + 1], &arg_tail, 10);
    
    if (arg_tail[0] != '\0')
    {
      cout << "Invalid Usage" << endl;
      print_usage();
      return 1;
    }
    ++i;
  }

  //okay, we're good with args, lets start calculating
  registerSignalHandlers();

  float currentTemperature = (float)initial_temperature;

  //create message queue 
  msgQ = new MessageQueue(id);

  //create message queue clients
  MessageQueueClient<StatusMessage> agentStatusQ(id);
  MessageQueueClient<StatusMessage> n1StatusQ(n1Id);
  MessageQueueClient<StatusMessage> n2StatusQ(n2Id);

  //wait for queues to come online....
  agentStatusQ.attachToQueue(-1);
  n1StatusQ.attachToQueue(-1);
  n2StatusQ.attachToQueue(-1);
  bool stabilized = false;

  //setup initial status for neighbors
  StatusMessage n1Status = {INITIALIZED, currentTemperature};
  StatusMessage n2Status = n1Status;
  
  //Send initial status and temperatur to neighbors
  StatusMessage agentStatus = {INITIALIZED, currentTemperature};
  n1StatusQ.sendMessage(agentStatus, id);
  n2StatusQ.sendMessage(agentStatus, id);
  

  while (!stabilized)
  {
    if (n1Status.status != STABILIZED)
    {
      //read from neighbor 1
      try
      {
        n1Status = agentStatusQ.readMessage(n1Id, false); 
      } catch (QueueConnectionException qce)
      {
        cout << id << ": qce exception" << endl;
      }
    }

    if (n2Status.status != STABILIZED)
    {
      //read from neighbor 2
      try
      {
        n2Status = agentStatusQ.readMessage(n2Id, false); 
      } catch (QueueConnectionException qce)
      {
        cout << id << ": qce exception" << endl;
      }
    }
  
    //Calculate new temperature 
    float newTemperature = currentTemperature 
                           + (0.33 * (n1Status.temperature - currentTemperature))
                           + (0.33 * (n2Status.temperature - currentTemperature));

    //calculate how stable the temp is
    float temperatureStabilization = fabs(newTemperature - currentTemperature);
    
    cout << id << ": temp was " << currentTemperature << " and will be set to " << newTemperature << endl;
    currentTemperature = newTemperature;
    
    if (temperatureStabilization < TEMP_THRESHOLD)
    {
      //we've reached a stabil temperature, carry on
      stabilized = true;
    }
    
    agentStatus = (StatusMessage) {INITIALIZED, currentTemperature};
    
    try
    {
      n1StatusQ.sendMessage(agentStatus, id);
    } catch (QueueConnectionException qce){}
    try
    {
      n2StatusQ.sendMessage(agentStatus, id);
    } catch (QueueConnectionException qce){}
    
    usleep(SLEEP_TIME); 
  }

  //Try to send messages to neighbors indicating stabilization
  //This may fail as the neighbor may have already shutdown its message queue
  agentStatus = (StatusMessage) {STABILIZED, currentTemperature};
  try
  {
    n1StatusQ.sendMessage(agentStatus, id);
  }
  catch (QueueConnectionException qce){}
  
  try
  {
    n2StatusQ.sendMessage(agentStatus, id);
  }
  catch (QueueConnectionException qce){}

  cout << "Agent " << id << " started with temp " << initial_temperature << " and stabilized to " << currentTemperature << endl;
  
  deleteMsgQ();

  return 0;
}
