//理发师进程
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
struct sembuf P,V;
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
    //printf("send from %d to %d is\n",my_pid,des);
}
void msgreceive(int des)
{
    msgrcv(msgid,&msg,sizeof(int)*3,des,0);
    int *temp=(int*)msg.mtext;
    get_pid=temp[0];
    state=temp[1];
    get_index=temp[2];
    //printf("receive from %d to %d\n",des,my_pid);
}
void ok_function()
{
    printf("等待顾客...\n");
    msgreceive(4);
    msgsend(get_pid,OK,my_index);
    printf("为编号为%d的顾客理发...\n",get_index);
    getchar();
    msgsend(get_pid,CHARGE,my_index);
    printf("理发完毕，结帐...\n");
    semop(monid,&P,1);
    //结帐
    semop(monid,&V,1);
    msgsend(get_pid,END,my_index);
    printf("结帐完毕，送作顾客\n");
}
main(int argc,char *argv[])
{
    name=(argv[1]==NULL)?"\0":argv[1];
    msgid=msgget(MSGKEY,0777|IPC_CREAT);
    my_pid=getpid();
    printf("%d\n",my_pid);
    P.sem_num=0;
    P.sem_op=-1;
    P.sem_flg=SEM_UNDO;
    V.sem_num=0;
    V.sem_op=1;
    V.sem_flg=SEM_UNDO;
    monid=semget(MONKEY,1,0666|IPC_CREAT);
    sen.val=1;
    semctl(monid,0,SETVAL,sen);     //SETVAL设置信号量集中的一个单独的信号量的值
    printf("我是理发师%s我正在找理发店工作\n",name);
    msgsend(1,CUTTER,0);
    msgreceive(my_pid);
    my_index=get_index;
    if(state==FULL) 
        printf("离开...\n");
    else if(state==OK)
    {
        printf("加入了一个理发店...\n");
        while(1) ok_function();
    }
}
