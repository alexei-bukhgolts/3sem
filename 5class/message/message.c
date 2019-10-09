#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/random.h>

typedef FILE File;

#define FIFO_12_PATH "/tmp/12.fifo"
#define FIFO_21_PATH "/tmp/21.fifo"

int fileExists(char* fname) {
  return access(fname, F_OK) != -1;
}

createFifoIfNotExists(char* fname) {
  if (!fileExists(fname))
    mkfifo(fname, 0666);
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    puts("Please select mode");
    return 1;
  }
  File* writeFifo;
  File* readFifo;
  createFifoIfNotExists(FIFO_12_PATH);
  createFifoIfNotExists(FIFO_21_PATH);
  if (argv[1][0] == '1') {
    puts("Type 1");
    writeFifo = fopen(FIFO_12_PATH, "w");
    readFifo = fopen(FIFO_21_PATH, "r");
  }
  else {
    puts("Type 2");
    readFifo = fopen(FIFO_12_PATH, "r");
    writeFifo = fopen(FIFO_21_PATH, "w");
  }
  puts("Fifo init complete");
  pid_t forkid = fork();
  char buf[1000];
  while (1) {
    if (forkid != 0) {
      fgets(buf, 1000, stdin);
      fputs(buf, writeFifo);
      fflush(writeFifo);
    }
    else {
      char* res = fgets(buf, 1000, readFifo);
      if (res == NULL) return 0;
      printf(">>> %s", buf);
    }
  }
  // remove((argv[1][0] == '1' ? "/tmp/12.fifo" : "/tmp/21.fifo");
  return 0;
}
