#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int fds[2];
  pipe(fds);

  int pid = fork();
  if (pid == 0) {
    // child 
    uint8 current_pid;
    read(fds[0], &current_pid, 1);
    printf("%d: received ping\n", current_pid);

    current_pid = getpid();
    write(fds[1],&current_pid, 1);
  } else {
    // father 
    uint8 current_pid = getpid();
    write(fds[1],&current_pid, 1);

    read(fds[0], &current_pid, 1);
    printf("%d: received pong\n", current_pid);
  }

  exit(0);
}
