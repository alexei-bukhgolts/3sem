#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define STR_COPY TRUE // Set to 0 if split(strin, ...) should modify the string
#define SINGLE_MALLOC_SIZE 1024
#define BUFFER_SIZE 16384

// If STR_COPY is set to true, stores tokens in a single allocated buffer
// else stores tokens as-is in the modified string
// The first token is actually string itself so that it can be free'd
void split(char* string, const char* delimiters, char*** tokens,
   int* tokensCount) {
    if(STR_COPY) {
      string = strdup(string);
    }
    char** tokenArray = (char**)malloc(sizeof(char*) * SINGLE_MALLOC_SIZE);  // number of tokens is unknown
    tokenArray[0] = string;
    tokenArray[1] = strtok(string, delimiters);
    for (size_t i = 2;; i++) { // double semicolon is not a typo
      if (i % SINGLE_MALLOC_SIZE == 0) { // if tokenArray is full
        tokenArray = (char**)realloc(tokenArray,
           (i + SINGLE_MALLOC_SIZE) * sizeof(char**));
      }
      char* token = strtok(NULL, delimiters);
      if (token == NULL) {
        *tokensCount = i;
        break;
      }
      tokenArray[i] = token;
    }
    *tokens = tokenArray;
}

int main(int argc, char const *argv[]) {
  char* delimiters = " #\t";
  char** tokens = NULL;
  char buffer[BUFFER_SIZE];
  int n = -1;
  fgets(buffer, BUFFER_SIZE, stdin);
  split(buffer, delimiters, &tokens, &n);
  for (size_t i = 1; i < n; i++) {
    puts(tokens[i]);
  }
  if (STR_COPY) free(tokens[0]); // frees token buffer if it exists
  free(tokens); // frees token pointers array
  return 0;
}
