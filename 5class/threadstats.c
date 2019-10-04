/*
  Acceleration of variance and mean computation with 1e9 data entries with multiple threads
  is, ahem, negative. It seems that this computation is too fast compared to duration of
  thread creation. To address that problem, one usually uses a thread pool. However, it is
  too complicated and not really necessary.

  Graph of program overall acceleration (6.7GB of data) is fig.png
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define BATCH_SIZE (1024*1024*5L)
#define BATCH_COUNT 128

typedef struct {
  int start;
  double** batches;
  int N_THREADS;
  double* results;
  double (*funPtr)(double* batch);
} threadFunctionArg;

struct timespec timer_start(){
    struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    return start_time;
}

// Copypasta from https://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
// call this function to end a timer, returning nanoseconds elapsed as a long
long timer_end(struct timespec start_time){
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    long diffInNanos = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 + (end_time.tv_nsec - start_time.tv_nsec);
    return diffInNanos;
}

double fillBatchWithRandomData(double* batch) {
  batch[0] = (double)random();
  for (size_t i = 0; i < BATCH_SIZE; i++) {
    batch[i] = 5 + ((double)(rand_r((unsigned int*)batch) / 200000000));
    // This is, to put it blantly, an ugly hack.
    // We use it to deal in a quick and dirty way with rand() and random() lack of thread safety
  }
  return 0;
}

double sumOfButch(double* batch) {
  double sum = 0;
  for (size_t i = 0; i < BATCH_SIZE; i++) {
    sum += batch[i];
  }
  return sum;
}

double sumOfButchSq(double* batch) {
  double sum = 0;
  for (size_t i = 0; i < BATCH_SIZE; i++) {
    sum += batch[i] * batch[i];
  }
  return sum;
}

void* threadFunction(void* argPtr) {
  threadFunctionArg arg = *(threadFunctionArg*)argPtr;
  for (size_t i = arg.start; i < BATCH_COUNT; i += arg.N_THREADS) {
    arg.results[i] = (*arg.funPtr)(arg.batches[i]);
  }
  return NULL;
}

double* multithreadedMap(double** batches, double (*funPtr)(double* batch), int N_THREADS) {
  double* results = (double*)calloc(BATCH_COUNT, sizeof(double));
  pthread_t* pThreads = (pthread_t*)calloc(N_THREADS, sizeof(pthread_t));
  threadFunctionArg* args = (threadFunctionArg*)calloc(BATCH_COUNT, sizeof(threadFunctionArg));
  for (int i = 0; i < N_THREADS; i++) {
    args[i] = (threadFunctionArg){.start = i, .batches = batches,
       .N_THREADS = N_THREADS, .results = results, .funPtr = funPtr};
    pthread_create(pThreads + i, NULL, threadFunction, &args[i]);
  }
  for (int i = 0; i < N_THREADS; i++) {
    pthread_join(pThreads[i], NULL);
  }
  free(args);
  free(pThreads);
  return results;
}

double** generateData(const int N_THREADS) {
  srand(time(NULL));
  double** batches = (double**)malloc(BATCH_COUNT * sizeof(double*));
  for (int i = 0; i < BATCH_COUNT; i++) {
    batches[i] = (double*)calloc(BATCH_SIZE, sizeof(double));
  }
  void* ptr = multithreadedMap(batches, fillBatchWithRandomData, N_THREADS);
  free(ptr);
  return batches;
}

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    puts("Please specify number of threads");
    return 1;
  }
  const int N_THREADS = atoi(argv[1]);
  double** batches = generateData(N_THREADS);
  struct timespec begin = timer_start();

  // for (size_t i = 0; i < BATCH_COUNT; i++) {
  //   for (size_t j = 0; j < BATCH_SIZE; j++) {
  //     printf("%lf ", batches[i][j]);
  //   }
  //   puts("");
  // }

  double* sums = multithreadedMap(batches, sumOfButch, N_THREADS);
  double fullSum = 0;
  for (size_t i = 0; i < BATCH_COUNT; i++) {
    fullSum += sums[i];
  }
  free(sums);

  double* sumsOfSq = multithreadedMap(batches, sumOfButchSq, N_THREADS);
  double fullSumOfSq = 0;
  for (size_t i = 0; i < BATCH_COUNT; i++) {
    fullSumOfSq += sumsOfSq[i];
  }
  free(sumsOfSq);

  long long nsElapsed = timer_end(begin);

  double mean = fullSum / (BATCH_SIZE * BATCH_COUNT);
  double meanOfSq = fullSumOfSq / (BATCH_SIZE * BATCH_COUNT);
  printf("Sum of all data entries is %lf\n", fullSum);
  printf("Sum of all squares is %lf\n", fullSumOfSq);
  printf("Mean is %lf\n", mean);
  printf("Mean of squares is %lf\n", meanOfSq);
  printf("Square of mean is %lf\n", mean * mean);
  printf("Variance is %lf\n", meanOfSq - mean * mean);
  printf("Nanoseconds after data generated: %lld\n", nsElapsed);

  for (int i = 0; i < BATCH_COUNT; i++) {
    free(batches[i]);
  }
  free(batches);
  return 0;
}
