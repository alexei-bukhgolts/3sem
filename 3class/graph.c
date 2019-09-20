#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int n = 0;

void spawnChildren(int* tree, int myId) {
  for (int i = 0; i < n; i++) {
    if (tree[i] == myId) {
      pid_t res = fork();
      if (res == 0) {
        spawnChildren(tree, i);
        return;
      }
      else if (res > 0) {
        printf("Process spawned, \
        tree id: %d\t parent tree id: %d\t pid: %d\t ppid: %d\n",
        i, myId, res, getpid());
      }
      else {
        puts("Error");
        return;
      }
    }
  }
}

// This program spawns a tree of processes.
// Input: int n (number of nodes in the tree), tree itself.
// Tree input is a list of whitespace separated node IDs.
// These are considered parents of the corresponding node.
// ID of the root node is -1, non-root IDs start at 0
// For example, input:
//   5
//   1 2 -1 -1 1
// gives the following tree:
//     -1
//   2    3
//   1
// 0  4
int main(int argc, char const *argv[]) {
  pid_t rootPid = getpid();
  scanf("%d", &n);
  int* tree = (int*)malloc(sizeof(int) * n);
  for (int* ptr = tree; ptr < tree + n; ptr++) {
    scanf("%d", ptr);
  }
  spawnChildren(tree, -1);
  free(tree);
  pid_t wpid;
  int status = -1;
  while ((wpid = wait(&status)) > 0);
  if (getpid() == rootPid)
    puts("All non-root processes terminated. Wait for me, children...");
  return 0;
}
