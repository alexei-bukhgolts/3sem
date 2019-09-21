// Compile and link with -pthread.
// Input is the "inp.txt" file (End-Of-Line terminated)
// Actually, using shell is better for reading program arguments as it allows
// to specify space-containing arguments easily.
// Maybe I will implement it in some time. Maybe not.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <wait.h>

#define TIMEOUT 5
#define SINGLE_MALLOC_SIZE 1024
#define BUFFER_SIZE 4096

typedef struct executeeEntry {
  pid_t pid;
  int startDelaySeconds;
} executeeEntry;

char lastChr(char *c) {
    if(c == NULL || *c == '\0') return 0;
    return c[strlen(c)-1];
}

void split(char* string, const char* delimiters, char*** tokens,
   int* tokensCount) {
    char** tokenArray = (char**)malloc(sizeof(char*) * SINGLE_MALLOC_SIZE);  // number of tokens is unknown
    tokenArray[0] = string;
    tokenArray[1] = strtok(string, delimiters);
    for (size_t i = 2;; i++) { // double semicolon is not a typo
      if (i % SINGLE_MALLOC_SIZE == 0) { // if tokenArray is full
        tokenArray = (char**)realloc(tokenArray,
           (i + SINGLE_MALLOC_SIZE) * sizeof(char**));
      }
      char* token = strtok(NULL, delimiters);
      tokenArray[i] = token;
      if (token == NULL) {
        *tokensCount = i;
        break;
      }
    }
    *tokens = tokenArray;
}

void* waitAndKill(void* executeeEntryPtrVoid) {
  executeeEntry* ee = (executeeEntry*)executeeEntryPtrVoid;
  sleep(ee->startDelaySeconds);
  pid_t pid = ee->pid;
  time_t startTime = time(NULL); // time() is not very precise but it's enough for us
  while (time(NULL) < startTime + TIMEOUT) {
    // We will check the state of the process periodically.
    // It is not the best idea but it is good enough for this use case
    nanosleep((const struct timespec[]){{0, 10L * 1000000L}}, NULL); // 10 ms
    if (waitpid(pid, NULL, WNOHANG) != 0) return NULL;
  }
  printf("Timeout of process %d exceeded. Aborting...\n", pid);
  kill(pid, SIGTERM);
  kill(pid, SIGKILL);
  free(executeeEntryPtrVoid);
  return NULL;
}

pthread_t executeFile(char** tokens) {
  pid_t res = fork();
  int delay = atoi(tokens[1]);
  if (res == 0) {
    sleep(delay);
    execvp(tokens[2], tokens + 2);
  }
  else {
    executeeEntry* executeeEntryPtr =
        (executeeEntry*)malloc(sizeof(executeeEntry*));
    executeeEntryPtr->pid = res;
    executeeEntryPtr->startDelaySeconds = delay;
    pthread_t threadId;
    pthread_create(&threadId, NULL, waitAndKill, executeeEntryPtr);
    return threadId;
  }
  return -1; // Should never happen
}

int main(int argc, char const *argv[]) {
  const char delim[] = "\t \n"; // ASCII only
  FILE* file = fopen("inp.txt", "rt");
  if (file == NULL) {
    puts("No inp.txt file found");
    exit(2);
  }
  char buf[BUFFER_SIZE];
  pthread_t threads[BUFFER_SIZE] = {0}; // BUFFER_SIZE is just for example.
  pthread_t* threadPtr = threads;
  while (fgets(buf, BUFFER_SIZE - 1, file) != NULL &&
      threadPtr - threads < BUFFER_SIZE) {
    if(lastChr(buf) != '\n') {
      puts("Error, line is too long");
      return 1;
    }
    // printf("Executing '%s'", buf);
    char** tokens = NULL;
    int n = -1;
    split(buf, delim, &tokens, &n);
    if (n > 2) {
      *threadPtr = executeFile(tokens);
      threadPtr++;
    }
    free(tokens);
  }
  fclose(file);
  for (pthread_t* joinedThreadPtr = threads; joinedThreadPtr < threadPtr;
        joinedThreadPtr++) {
    pthread_join(*joinedThreadPtr, NULL);
  }
  puts("All processes terminated. Closing executor...");
  return 0;
}
