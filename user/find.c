#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *path, char *name)
{
  int fd;
  struct stat st;
  struct dirent de;
  char file_path_buff[128];
  int file_path_buff_len = strlen(path);
  strcpy(file_path_buff, path);

  if((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type) {
  case T_FILE:
    write(1, "find: expect a dir\n", 19);
    break;

  case T_DIR:
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
      if(de.inum == 0)
        continue;

      char file_name_buff[DIRSIZ + 1];
      file_name_buff[DIRSIZ] = '\0';
      memcpy(file_name_buff, de.name, DIRSIZ);

      if(strcmp(file_name_buff, ".") == 0 || strcmp(file_name_buff, "..") == 0) {
        continue;
      }

      // fprintf(2, "file: %s\n", file_name_buff);
      // 拼接路径
      file_path_buff[file_path_buff_len] = '/';
      memcpy(file_path_buff + file_path_buff_len + 1, file_name_buff, strlen(file_name_buff) + 1);
      int sub_fd = open(file_path_buff, 0);
      fstat(sub_fd, &st);
      // important
      close(sub_fd);

      switch(st.type) {
      case T_FILE:
        if(strcmp(file_name_buff, name) == 0) {
          printf("%s\n", file_path_buff);
        }
        break;
      case T_DIR:
        // printf("finding in path: %s\n", file_path_buff);
        find(file_path_buff, name);
        break;
      }
    }
    break;
  }

  close(fd);
}


int
main(int argc, char *argv[])
{
  find(argv[1], argv[2]);

  exit(0);
}
