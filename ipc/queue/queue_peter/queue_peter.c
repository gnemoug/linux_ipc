#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<fcntl.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>

#define PROJID 0xFF
#define LUCY 1
#define PETER 2

int main()
{
    char filenm[] = "queue_lucy.c";
    int mqid;
    key_t mqkey;
    struct msgbuf 
    {
        long mtype;     /* message type, must be > 0 */
        char mtext[256]; /* message data */
    }msg;
    int ret;

    mqkey = ftok(filenm, PROJID);
    if(mqkey == -1) 
    {
       perror("ftok error: ");
       exit(-1);
    }

    mqid = msgget(mqkey, 0);
    if(mqid == -1) 
    {
       perror("msgget error: ");
       exit(-1);
    }

    while(1) 
    {
       msgrcv(mqid,&msg,256,LUCY,0);
       printf("Lucy: %s\n",msg.mtext);
       printf("Peter: ");
       fgets(msg.mtext,256,stdin);
       if(strncmp("quit", msg.mtext, 4) == 0) 
       {
            exit(0);
       }
       msg.mtext[strlen(msg.mtext)-1] = '\0';
       msg.mtype = PETER;
       msgsnd(mqid,&msg,strlen(msg.mtext) + 1,0);
    } 
}
