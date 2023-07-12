#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void 
find(char *path, const char *filename)
{
  char buf[512], *p;
  int fd;
  // 声明与文件相关的结构体
  struct dirent de;//inode,
  struct stat st;//文件状态信息

  //0为只读 打开成功为0 失败-1 返回文件描述符
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  //fstat() 函数的返回值为 0 表示成功，-1 表示失败
  //fd 是文件描述符，用于指定要获取信息的文件。
  //st 是一个指向 struct stat 结构的指针，用于存储获取的文件状态信息。
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot fstat %s\n", path);
    close(fd);
    return;
  }

  //参数错误，find的第一个参数必须是目录
  if (st.type != T_DIR) {
    fprintf(2, "usage: find <DIRECTORY> <filename>\n");
    return;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
    fprintf(2, "find: path too long\n");
    return;
  }
  strcpy(buf, path);//目录路径复制入buf
  p = buf + strlen(buf); 
  *p++ = '/'; //p指向最后一个'/'之后//形成绝对路径
  while (read(fd, &de, sizeof de) == sizeof de) {
    if (de.inum == 0)
      continue;
    memmove(p, de.name, DIRSIZ); //添加路径名称
    p[DIRSIZ] = 0;               //字符串结束标志
    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }
    //不要在“.”和“..”目录中递归
    if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
      find(buf, filename);
    } 
    else if (strcmp(filename, p) == 0){
      printf("%s\n", buf);
    }
  }

  close(fd);
}

int 
main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(2, "usage: find <directory> <filename>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}