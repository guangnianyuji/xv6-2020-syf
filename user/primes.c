#include "kernel/types.h"
#include "user/user.h"

#define RD 0
#define WR 1

//子进程是原始进程的副本，包括代码、数据、堆栈和文件描述符等。

void 
trans(int fd_l[2])
{

    int fd_r[2];
    pipe(fd_r);
    close(fd_l[WR]);

    int ans;
    int res=read(fd_l[RD],&ans,sizeof(int));
  
    if(res==sizeof(int)){
        fprintf(1,"prime %d\n",ans);
    }
    else{
        exit(0);
    }
 
    int data;
    while(read(fd_l[RD],&data,sizeof(int))==sizeof(int)){
        if(data%ans!=0){
            write(fd_r[WR],&data,sizeof(int));
        }
    }

    int pid=fork();
    if(pid==0){//是子进程
        trans(fd_r);
    }
    else{
        close(fd_r[RD]);
        close(fd_r[WR]);
        wait(0);//参数的意义？？
    }
    exit(0);
}

int 
main(int argc, char const *argv[])
{
    int fd_r[2];
    pipe(fd_r);//初始化通道的写端（右端r）  
     
    for(int i=2;i<=35;i++){
        write(fd_r[WR],&i,sizeof(int));
    }

    int pid=fork();

    if(pid==0){//是子进程
        trans(fd_r);
    }
    else{
        close(fd_r[RD]);
        close(fd_r[WR]);
        wait(0);//参数的意义？？参数为0的意义是等待任意子进程结束。
    }
    exit(0);
}