#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define RD 0 //pipe的read端
#define WR 1 //pipe的write端

/*
标准输入（Standard Input）：通常用文件描述符 0 表示。
标准输出（Standard Output）：通常用文件描述符 1 表示。
标准错误输出（Standard Error）：通常用文件描述符 2 表示。
*/

int 
main(int argc, char const *argv[]) 
{
    char buf = 'P'; //用于传送的字节

    int fd_c2p[2]; //子进程->父进程
    int fd_p2c[2]; //父进程->子进程
    /*内核会为管道分配两个文件描述符
     （文件描述符是用于访问文件或其他I/O资源的整数）
    分别用于读取和写入管道。
    0用于读取数据，1用于写入数据
    */
    pipe(fd_c2p);
    pipe(fd_p2c);

    int pid = fork();
    int exit_status = 0;

    if (pid < 0) {
        fprintf(2, "fork() error!\n");
        close(fd_c2p[RD]);
        close(fd_c2p[WR]);
        close(fd_p2c[RD]);
        close(fd_p2c[WR]);
        exit(1);
    } 
    else if (pid == 0) { //子进程
    //如果写端没有关闭，读取会阻塞
        close(fd_p2c[WR]);
        close(fd_c2p[RD]);

        //文件描述符、指向缓冲区的指针(用于存储读取的数据)、要读取的最大字节数
        if (read(fd_p2c[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "child read() error!\n");
            exit_status = 1; //标记出错
        } else {
            fprintf(1, "%d: received ping\n", getpid());
        }

        if (write(fd_c2p[WR], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "child write() error!\n");
            exit_status = 1;
        }

        close(fd_p2c[RD]);
        close(fd_c2p[WR]);

        exit(exit_status);
    } 
    else { //父进程
        close(fd_p2c[RD]);
        close(fd_c2p[WR]);

        if (write(fd_p2c[WR], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "parent write() error!\n");
            exit_status = 1;
        }

        if (read(fd_c2p[RD], &buf, sizeof(char)) != sizeof(char)) {
            fprintf(2, "parent read() error!\n");
            exit_status = 1; //标记出错
        } else {
            fprintf(1, "%d: received pong\n", getpid());
        }

        close(fd_p2c[WR]);
        close(fd_c2p[RD]);

        exit(exit_status);
    }
}