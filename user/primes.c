#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
str_rev(char *buff, int len)
{
  for(int idx = 0; idx < len / 2; idx++) {
    char tmp = buff[idx];
    buff[idx] = buff[len - 1 - idx];
    buff[len - 1 - idx] = tmp;
  }
}

int
itoa(int n, char *dest)
{
  *dest = n % 10 + '0';
  if(n < 10) {
    return 1;
  } else {
    int past_len = itoa(n / 10, dest + 1);
    return past_len + 1;
  }
}

void
print_int(int n)
{
  char buff[10];
  int len = itoa(n, buff);
  str_rev(buff, len);
  write(1, buff, len);
  write(1, "\n", 1);
}

void
make_child(uint8 is_last, int up_pipe_fds[2], int down_pipe_fds[2], int current_number)
{
  write(1, "prime ", 6);
  print_int(current_number);
  close(up_pipe_fds[1]);

  int input;
  while(read(up_pipe_fds[0], &input, 4) != 0) {
    // print_int(input);
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
  // write(1, "child return\n", 13);
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
