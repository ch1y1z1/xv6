#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int time;

  if(argc != 2) {
    write(1, "expect 1 arg\n", 14);
    exit(1);
  }
  time = atoi(argv[1]);

  sleep(time);

  exit(0);
}