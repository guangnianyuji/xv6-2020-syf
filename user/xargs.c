#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define STDIN 0
#define MAXARG 32  // max exec arguments
#define MAXBUF 512
int 
main(int argc, char *argv[])
{
    char *cmd;
    char *params[MAXARG];
    char buf[MAXBUF];
    if(argc<2){
        fprintf(2,"Usage: xargs <command>\n");
        exit(0);
    }
    if (argc + 1 > MAXARG){
        fprintf(2, "too many args\n");
        exit(0);
    }
    cmd=argv[1];

    for(int i=1;i<argc;i++){
        params[i-1]=argv[i];
    }

    int index=0;

    //读标准输入
    while(1){
        index=0;
        while(1){
            int n=read(STDIN,&buf[index],1);
            if(n==0){
                break;
            }
            if(buf[index]=='\n'){
                break;
            }
            index++;
        }
        if(index==0){
            break;
        }
        buf[index]=0;
        params[argc-1]=buf;
         int pid=fork();
         if(pid==0){
            exec(cmd,params);
            exit(0);
         }
         else{
            wait(0);
         }
    }
    exit(0);
}