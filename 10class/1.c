#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SOLVE 1

#define CLIENT_COUNT 5
#define SERVER_COUNT 2

long id;
int msqid;


typedef struct {
  long id;
  long a;
  long b;
} MessagePayload;

typedef struct {
   long mtype;
   MessagePayload data;
} Message;

void client() {
  Message message;
  while (1) {
    long a, b;
    a = rand() >> 26;
    b = rand() >> 26;
    message.mtype = SOLVE;
    message.data.a = a;
    message.data.b = b;
    message.data.id = id;
    msgsnd(msqid, &message, sizeof(MessagePayload), 0);
    msgrcv(msqid, &message, sizeof(MessagePayload), id, 0);
    long res = message.data.a;
    printf("%d %d %d\n", a, b, res);
    sleep(1);
  }
}

void server() {
  Message message;
  while(1) {
    msgrcv(msqid, &message, sizeof(MessagePayload), SOLVE, 0);
    long messageType = message.data.id;
    long a = message.data.a;
    long b = message.data.b;
    message.data.a = a * b;
    message.mtype = message.data.id;
    // printf("%d %d %d\n", a, b, id);
    msgsnd(msqid, &message, sizeof(MessagePayload), 0);
  }
}

int main(void)
{
    char pathname[] = "desc";
    key_t  key;

    /* Create or attach message queue  */

    key = ftok(pathname, 0);

    if ((msqid = msgget(key, 0666 | IPC_CREAT)) < 0){
       printf("Can\'t get msqid\n");
       exit(-1);
    }


    for (size_t i = 0; i < CLIENT_COUNT; i++) {
      int pid = fork();
      srandom(pid);
      if (pid == 0)
        continue;
      id = pid;
      client();
      return 0;
    }

    for (size_t i = 0; i < SERVER_COUNT; i++) {
      id = i;
      int pid = fork();
      if (pid == 0)
        continue;
      server();
      return 0;
    }

    return 0;
}
