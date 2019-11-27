#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <sys/msg.h>
#include <errno.h>

int pid;
char byteReceived = 0;
char bitN = 0;

void handler(int signum) {
  byteReceived |= (((signum == SIGUSR2) ? 1 : 0) << bitN);
  bitN++;
  if (bitN == 8) {
    putc(byteReceived, stdout);
    fflush(stdout);
    bitN = 0;
    byteReceived = 0;
  }
}

void sendBit(const char* s, int n) {
  char bit = (s[n / 8] >> (n % 8)) & 1;

  // printf("%d\n", bit);
  kill(pid, bit ? SIGUSR2 : SIGUSR1);
}

void client() {
  usleep(1);
    /*
        Without this usleep() many signals are ignored. This lame type of
        synchronization may seem a bad idea, and surely it is.
        However, the idea of sending 1-bit messages via signals is not
        brilliant, either!
    */
  const char* info = "I love Emilia!\n";
  int i = 0;
  while(info[i / 8]) {
    sendBit(info, i);
    i++;
    usleep(1);
  }
  kill(pid, SIGTERM);
}

void server() {
  while (1) {}
}

int main(int argc, char const *argv[]) {
  setbuf(stdout, NULL);
  pid = fork();
  signal(SIGUSR1, handler);
  signal(SIGUSR2, handler);
  if (pid != 0) client();
  else server();
  return 0;
}
