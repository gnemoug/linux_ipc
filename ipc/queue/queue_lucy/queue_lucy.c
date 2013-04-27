#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<fcntl.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>

#define PROJID 0xFF
#define LUCY 1
#define PETER 2

int mqid;

void terminate_handler(int signo)
{
    msgctl(mqid,IPC_RMID,NULL);     //用于处理消息队列
    exit(0);
}

int main()
{
    char filenm[] = "queue_lucy.c";
    key_t mqkey;
    struct msgbuf   //
    {
        long mtype;     /* message type, must be > 0 */
        char mtext[256]; /* message data */
    }msg;
    int ret;

    mqkey = ftok(filenm,PROJID);
    if(mqkey == -1) 
    {
       perror("ftok error: ");
       exit(-1);
    }

    mqid = msgget(mqkey,IPC_CREAT | IPC_EXCL | 0666);   //初始化消息队列
    if(mqid == -1) 
    {
       perror("msgget error: ");
       exit(-1);
    }

    signal(SIGINT, terminate_handler);  //处理ctrl+c信号
    signal(SIGTERM, terminate_handler);     //kill命令信号

    while(1)
    {
       printf("Lucy: ");
       fgets(msg.mtext, 256, stdin);    //从标准输入流输入数据
       if (strncmp("quit", msg.mtext, 4) == 0) 
       {
            msgctl(mqid,IPC_RMID,NULL);
            exit(0);
       }
       msg.mtext[strlen(msg.mtext)-1] = '\0';
       msg.mtype = LUCY;
       msgsnd(mqid,&msg,strlen(msg.mtext) + 1,0);
       msgrcv(mqid,&msg,256,PETER,0);       //若不存在则阻塞
       printf("Peter: %s\n", msg.mtext);
    }  
}
