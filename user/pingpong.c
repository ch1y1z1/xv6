#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

uint8 u8toa(uint8 n, char* dest) {
  *dest = n % 10 + '0';
  if (n < 10) {
    return 1;
  } else {
    int past_len = u8toa(n / 10, dest + 1);
    return past_len + 1;
  }
}

void print_u8(uint8 n) {
  char buff[4];

  uint8 len = u8toa(n, buff);
  write(1, buff, len);
}

int
main(int argc, char *argv[])
{
  int ping_fds[2], pong_fds[2];
  if(pipe(ping_fds) != 0 || pipe(pong_fds) != 0) {
    write(1, "pipe error\n", 12);

    exit(1);
  }

  int pid = fork();
  if(pid == 0) {
    // child
    uint8 read_pid;
    read(ping_fds[0], &read_pid, 1);
    print_u8(read_pid);
    write(1, ": received ping\n", 16);

    uint8 write_pid = (uint8)getpid();
    write(pong_fds[1], &write_pid, 1);
  } else {
    // father
    uint8 write_pid = (uint8)getpid();
    write(ping_fds[1], &write_pid, 1);

    uint8 read_pid;
    read(pong_fds[0], &read_pid, 1);
    print_u8(read_pid);
    write(1, ": received pong\n", 16);
  }

  exit(0);
}
