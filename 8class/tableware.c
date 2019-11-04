#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define TABLEWARE_TYPES_N 20
#define DIRTY_ENTRIES_N 20

int TABLE_SIZE = 30;
int TABLEWARE_COUNT = 0;

typedef enum {
  WASHING_COMPLETE = 1,
  WIPING_COMPLETE = 2,
  DUMMY = LONG_MAX // to force enum to be long
} MessageType;

typedef struct {
  long globalTime;
  long tablewareType;
} MessagePayload;

typedef struct {
  MessageType mtype;
  MessagePayload data;
} Message;

typedef struct {
  long a;
  long b;
} longPair;

longPair washingTimes[TABLEWARE_TYPES_N] = {0};
longPair wipingTimes[TABLEWARE_TYPES_N] = {0};
longPair dirtyTableware[TABLEWARE_TYPES_N] = {0};

long getTime(long tablewareType, int isWiping) { // Otherwise washing
  longPair* array = isWiping ? wipingTimes : washingTimes;
  for (size_t i = 0; i < TABLEWARE_TYPES_N; i++) {
    if (array[i].a == tablewareType) return array[i].b;
  }
  return 0;
}

void washer(int qid) {
  long globalTime = 0;
  long freePositionsOnTable = TABLE_SIZE;
  Message msg;
  int tablewareCount = TABLEWARE_COUNT;
  longPair* dirtyPtr = dirtyTableware;
  long washingTime;
  while (tablewareCount > 0) {
    while (freePositionsOnTable > 0) {
      while (dirtyPtr->b == 0) {
        dirtyPtr += 1;
        washingTime = getTime(dirtyPtr->a, 0);
      }
      (dirtyPtr->b)--;

      globalTime += washingTime;
      freePositionsOnTable--;
      tablewareCount--;
      printf("Washed! type:%ld, global time: %ld\n", dirtyPtr->a, globalTime);
      fflush(stdout); // Avoid mixing outputs
      msg.mtype = WASHING_COMPLETE;
      msg.data.globalTime = globalTime;
      msg.data.tablewareType = dirtyPtr->a;
      msgsnd(qid, &msg, sizeof(MessagePayload), 0);
      if (tablewareCount == 0) {
        printf("Washer has finished working at %ld.\n", globalTime);
        return;
      }
    }
    msgrcv(qid, &msg, sizeof(MessagePayload), WIPING_COMPLETE, 0);
    freePositionsOnTable++;
    globalTime = MAX(globalTime, msg.data.globalTime);
  }
}

void wiper(int qid) {
  long globalTime = 0;
  Message msg;
  int tablewareCount = TABLEWARE_COUNT;
  while(tablewareCount > 0) {
    msgrcv(qid, &msg, sizeof(MessagePayload), WASHING_COMPLETE, 0);
    globalTime = MAX(globalTime, msg.data.globalTime);
    globalTime += getTime(msg.data.tablewareType, 1);
    tablewareCount--;
    printf("Wiped: type:%ld, global time: %ld\n",
        msg.data.tablewareType, globalTime);
    fflush(stdout); // Avoid mixing outputs
    msg.mtype = WIPING_COMPLETE;
    msg.data.globalTime = globalTime;
    msgsnd(qid, &msg, sizeof(MessagePayload), 0);
  }
  printf("Wiper has finished working at %ld.\n", globalTime);
  return;
}

void fileInput() {
    long a, b, i = 0;
    FILE* washingTimesFile = fopen("washing-times.txt", "r");
    while (fscanf(washingTimesFile, "%ld:%ld", &a, &b) == 2) {
      washingTimes[i] = (longPair){.a = a, .b = b};
      i++;
    }
    fclose(washingTimesFile);

    FILE* wipingTimesFile = fopen("wiping-times.txt", "r");
    while (fscanf(wipingTimesFile, "%ld:%ld", &a, &b) == 2) {
      wipingTimes[i] = (longPair){.a = a, .b = b};
    }
    fclose(wipingTimesFile);

    FILE* dirtyFile = fopen("dirty.txt", "r");
    while (fscanf(wipingTimesFile, "%ld:%ld", &a, &b) == 2) {
      TABLEWARE_COUNT += b;
      dirtyTableware[i] = (longPair){.a = a, .b = b};
      i++;
    }
    fclose(dirtyFile);
}

int main(int argc, char const *argv[]) {
  char* TABLE_LIMIT = getenv("TABLE_LIMIT");
  if (TABLE_LIMIT == NULL) {
    puts("TABLE_LIMIT not specifed");
    return 1;
  }
  TABLE_SIZE = atol(TABLE_LIMIT);
  fileInput();

  key_t msg_queue_key = ftok("desc", 0);
  int qid = msgget(msg_queue_key, IPC_CREAT | 0666);
  int pid = fork();
  if (pid != 0) {
    wiper(qid);
    msgctl(qid, IPC_RMID, NULL);
  }
  else {
    washer(qid);
  }
  return 0;
}
