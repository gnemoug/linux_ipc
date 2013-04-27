//顾客进程
#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/sem.h>
#define MSGKEY  66
#define MONKEY  88
enum{CUTTER,CUSTOMERIN,CUSTOMERLEAVE,ROOM,SEAT,WORK,FULL,CHARGE,END,OK};
int my_pid;
int get_pid;
int state;
int my_index;
int get_index;
int msgid;
int monid;
int cutter;
int cutterindex;
int customer;
int customerindex;
char *name;
struct msgform{
    long mtype;
    char mtext[40];
}msg;
union senum{
    int val;
}sen;
void msgsend(int des,int state,int index)
{
    msg.mtype=des;
    int *temp=(int*)msg.mtext;
    temp[0]=my_pid;
    temp[1]=state;
    temp[2]=index;
    msgsnd(msgid,&msg,sizeof(int)*3,0);
}
void msgreceive(int des)
{
    msgrcv(msgid,&msg,sizeof(int)*3,des,0);
    int *temp=(int*)msg.mtext;
    get_pid=temp[0];
    state=temp[1];
    get_index=temp[2];
}
void call_seat()
{
    printf("找到一个理发店，我的编号是%d\n",get_index);
    if(state==ROOM)
    {
        printf("我来到大厅等候\n");
        msgsend(3,ROOM,my_index);
        msgreceive(my_pid);
    }
    if(state==SEAT)
    {
        printf("我来到座位上等候\n");
        msgsend(2,SEAT,my_index);
        msgreceive(my_pid);
    }
    if(state==WORK)
    {
        msgsend(4,WORK,my_index);
        msgreceive(my_pid);
        printf("编号为%d的理发师为我理发...\n",get_index);
        msgreceive(my_pid);
        printf("理发完毕，正在找钱...\n");
        msgsend(1,CUSTOMERLEAVE,my_index);
        msgreceive(my_pid);
        printf("找钱完毕，离开...\n");
    }
}
main(int argc,char *argv[])
{
    name=(argv[1]==NULL)?"\0":argv[1];
    msgid=msgget(MSGKEY,0666|IPC_CREAT);
    my_pid=getpid();
    printf("我是%s我正在寻找理发店理发\n",name);
    msgsend(1,CUSTOMERIN,0);
    msgreceive(my_pid);
    my_index=get_index;
    if(state==FULL) 
        printf("理发店人满，离开...\n");
    else 
        call_seat();
}
