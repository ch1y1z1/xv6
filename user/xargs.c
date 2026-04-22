#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
gets_chi(char *buf, int max)
{
  int i, cc;
  char c;

  for(i = 0; i + 1 < max;) {
    cc = read(0, &c, 1);
    if(cc < 1) {
      buf[i] = '\0';
      return cc;
    } else if(cc == 0) {
      buf[i] = '\0';
      return cc;
    }
    if(c == '\n' || c == '\r')
      break;
    buf[i++] = c;
  }
  buf[i] = '\0';
  return i;
}

int
main(int argc, char *argv[])
{
  char buff[128];
  int max_len = 128, get_len;

  while(get_len = gets_chi(buff, max_len), get_len > 0) {
    // make argvs
    char *new_argv[128];
    for(int idx = 1; idx < argc; idx++) {
      new_argv[idx - 1] = argv[idx];
    }
    new_argv[argc - 1] = buff;
    new_argv[argc] = 0;

    int pid = fork();
    if(pid == 0) {
      // child
      exec(argv[1], new_argv);
    } else {
      // father
      int status;
      wait(&status);
    }
  }

  exit(0);
}
