#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
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
int cutter;     //the number of cutter
int cutterindex;
int customer;    //the number of customer
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
    //printf("send from %d to %d\n",my_pid,des);
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
void cutter_function()
{
    if(cutter>=1) 
    msgsend(get_pid,FULL,0);
    else if(cutter>=0)
    {
        msgsend(get_pid,OK,++cutterindex);
        printf("一名理发师加入本店，为其编号%d\n",cutterindex);
        ++cutter;
    }
    else 
        printf("ERROR1\n");
}
void customerin_function()
{
    if(customer>=3) 
        msgsend(get_pid,FULL,0);
    else if(customer>=2)
    {
        msgsend(get_pid,ROOM,++customerindex);
        printf("一名顾客来到理发店在大厅等候，为其编号%d\n",customerindex);
        ++customer;
    }
    else if(customer>=1)
    {
        msgsend(get_pid,SEAT,++customerindex);
        printf("一名顾客来到理发店在座位上等候，为其编号%d\n",customerindex);
        ++customer;
    }
    else if(customer>=0)
    {
        msgsend(get_pid,WORK,++customerindex);
        printf("一名顾客来到理发店开始理发，为其编号%d\n",customerindex);
        ++customer;
    }
    else 
        printf("ERROR2\n");
}
void customerleave_function()
{
    printf("一名顾客离开本店，其编号为%d\n",get_index);
    printf("customer:%d\n",customer);
    if(customer>1)
    {
        msgreceive(2);
        msgsend(get_pid,WORK,get_index);
        printf("编号为%d的顾客从座位上坐起来去理发\n",get_index);
    }
    if(customer>2)
    {
        msgreceive(3);
        msgsend(get_pid,SEAT,get_index);
        printf("编号为%d的顾客从大厅坐到了座位上继续等待\n",get_index);
    }
    if(customer>3) 
        printf("ERROR3\n");
    --customer;
}
main(int argc,char *argv[])
{
    name=(argv[1]==NULL)?"\0":argv[1];
    my_pid=getpid();
    msgid=msgget(MSGKEY,0777|IPC_CREAT);    //用的不好，没有用到ftok去获取唯一键值
/*    创建消息队列*/
    while(1)
    {
        printf("目前有理发师%d名，顾客%d名\n",cutter,customer);
        msgreceive(1);
        if(state==CUTTER) 
            cutter_function();
        else if(state==CUSTOMERIN) 
            customerin_function();
        else if(state==CUSTOMERLEAVE) 
            customerleave_function();
    }
}
