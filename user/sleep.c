#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int 
main(int argc, char *argv[])
{
    if(argc <= 1){
    fprintf(2, "usage: sleep <time>\n");
    //文件描述符为2，它通常用于向用户显示错误消息或警告
    exit(1);
  }
   sleep(atoi(argv[1]));
   exit(0);
}