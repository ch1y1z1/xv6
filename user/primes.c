#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
make_child(uint8 is_last, int up_pipe_fds[2], int down_pipe_fds[2], int current_number)
{
  printf("prime %d\n", current_number);
  close(up_pipe_fds[1]);

  int input;
  while(read(up_pipe_fds[0], &input, 4) != 0) {
    if(input % current_number != 0) {
      if(is_last) {
        is_last = 0;
        int pid = fork();
        if(pid == 0) {
          // child
          close(up_pipe_fds[0]);
          int new_pipe_fds[2];
          pipe(new_pipe_fds);
          make_child(1, down_pipe_fds, new_pipe_fds, input);
        } else {
          // father
        }
      } else {
        write(down_pipe_fds[1], &input, 4);
      }
    }
  }

  close(down_pipe_fds[0]);
  close(down_pipe_fds[1]);
  if(!is_last) {
    int status;
    wait(&status);
  }
  
  close(up_pipe_fds[0]);
  exit(0);
}

int
main(int argc, char *argv[])
{
  uint8 is_last = 1;
  int up_pipe_fds[2], down_pipe_fds[2];

  pipe(up_pipe_fds);
  down_pipe_fds[0] = up_pipe_fds[0];
  down_pipe_fds[1] = up_pipe_fds[1];

  int pid = fork();
  if(pid == 0) {
    // child
    int new_pipe_fds[2];
    pipe(new_pipe_fds);
    make_child(is_last, up_pipe_fds, new_pipe_fds, 2);
  } else {
    // father
    close(down_pipe_fds[0]);
    for(int idx = 3; idx < 36; idx++) {
      // print_int(idx);
      write(down_pipe_fds[1], &idx, 4);
    }
  }

  close(down_pipe_fds[1]);
  int status;
  wait(&status);

  exit(0);
}
