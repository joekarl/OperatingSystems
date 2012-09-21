#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h> 
#include <sys/time.h>
#include <errno.h>

#define EXPECTED_ARGS_NUM 4
#define QUEUE_ID_OFFSET 12345

struct float_message
{
  long mtype;
  float temperature;
};

//return the size of a message minus its mtype
int sizeof_message();

//create a mailbox via an agent id
//returns -1 for failure or the mailbox id
int create_mailbox_for_agent(int agent_id);

//destroy a mailbox
void destroy_mailbox(int mailbox_id);

//attach to a mailbox for agent id
//will block until mailbox is available
//will timeout eventually if agent doesn't appear....
//if it timesout will return a -1 for failure
int attach_to_mailbox_for_agent(int agent_id);

//send a float message to agent
void send_message_to_mailbox_for_agent(float message, int agent_id);

//read a float message from a mailbox
float read_message_from_mailbox_for_agent(int agent_id);

//calculate if temperature is stable
//return 1 if yes 0 if no
int is_temperature_stable(float new_temperature, float current_temperature);

//calculate the new temperature based on neighbor agents
float calculate_new_temperature(float current_temperature, float neighbor_1_temperature, float neighbor_2_temperature);

void print_usage()
{
  printf("Usage -- agent <initial_temperature> <id> <neighbor_1_id> <neighbor_2_id>\n");
}

int main(int argc, char** argv)
{
  if (argc != EXPECTED_ARGS_NUM + 1) 
  {
    print_usage();
    return 1;
  }

  int initial_temperature, id, neighbor_1_id, neighbor_2_id;
  
  int* arg_order[EXPECTED_ARGS_NUM] = {&initial_temperature, 
                       &id, 
                       &neighbor_1_id, 
                       &neighbor_2_id};
  char* arg_tail;

  int i = 0;
  while(i < EXPECTED_ARGS_NUM)
  {
    (*(arg_order[i])) = strtol(argv[i + 1], &arg_tail, 10);
    
    if (arg_tail[0] != '\0')
    {
      print_usage();
      return 1;
    }
    ++i;
  }

  id += QUEUE_ID_OFFSET;
  neighbor_1_id += QUEUE_ID_OFFSET;
  neighbor_2_id += QUEUE_ID_OFFSET;

  printf("initialized agent %d....\n",id);

  int agent_mailbox_id = -1;
  //int agent_mailbox_id = 4161536;
  errno = 0;
  while (agent_mailbox_id == -1)
  {
    agent_mailbox_id = create_mailbox_for_agent(id);
  }

  int rtn = attach_to_mailbox_for_agent(id);  
  printf("attach to mailbox returned: %d\n", rtn);

  struct float_message msg = {1,1.5};
  int snd_rtn = 0;
  snd_rtn = msgsnd(agent_mailbox_id, &msg, sizeof_message(), 0);
  printf("sent message: %d errno: %d\n", snd_rtn, errno);

  struct float_message queue_msg;
  msgrcv(agent_mailbox_id, &queue_msg, sizeof_message(), 0, 0);
  printf("got message and it had a value of: %f\n", queue_msg.temperature);

  errno = 0;
  destroy_mailbox(agent_mailbox_id);
  printf("deleted mailbox: %d with errno:%d\n", agent_mailbox_id, errno);

  snd_rtn = msgsnd(agent_mailbox_id, &msg, sizeof_message(), 0);
  printf("sent message2: %d\n", snd_rtn);

  rtn = attach_to_mailbox_for_agent(id);  
  printf("attach to mailbox returned: %d\n", rtn);

  return 0;
}

int create_mailbox_for_agent(int agent_id)
{
  return msgget(agent_id, IPC_CREAT | 0400 | 0200);
}

void destroy_mailbox(int mailbox_id)
{
  //delete mq with id
  msgctl(mailbox_id, IPC_RMID, NULL);
}

int sizeof_message()
{
  static int message_size = sizeof(struct float_message) - sizeof(long);
  return message_size;
}

int attach_to_mailbox_for_agent(int agent_id)
{
  int mailbox_id = -1;
  struct timeval time;
  gettimeofday(&time, NULL);
  int curtime = time.tv_sec;
  int prevtime = curtime;
  while (mailbox_id == -1 && (curtime - prevtime) < 10)
  {
    gettimeofday(&time, NULL);
    curtime = time.tv_sec;
    mailbox_id = msgget(agent_id, IPC_NOWAIT);
  }
  return mailbox_id;
}


