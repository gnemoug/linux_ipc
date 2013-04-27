/*server.c:向共享内存中写入People*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

#include "credis.h"

int semid;
int shmid;

/*信号量的P操作*/
void p()
{
    struct sembuf sem_p;
    sem_p.sem_num=0;/*设置哪个信号量*/
    sem_p.sem_op=-1;/*定义操作*/   
    if(semop(semid,&sem_p,1)==-1)
        printf("p operation is fail\n");  
    /*semop函数自动执行信号量集合上的操作数组。 
　　 int semop(int semid, struct sembuf semoparray[], size_t nops); 
　　 semoparray是一个指针，它指向一个信号量操作数组。nops规定该数组中操作的数量。*/ 
}

/*信号量的V操作*/
void v()
{
    struct sembuf sem_v;
    sem_v.sem_num=0;
    sem_v.sem_op=1;
    if(semop(semid,&sem_v,1)==-1)
        printf("v operation is fail\n");
}

int main()
{
    struct People{
        char name[10];
        int age;
    };

    key_t semkey;
    key_t shmkey;
    semkey=ftok("../test/VenusDB.cbp",0);       //用来产生唯一的标志符，便于区分信号量及共享内存
    shmkey=ftok("../test/main.c",0);

    /*创建信号量的XSI IPC*/
    semid=semget(semkey,1,0666|IPC_CREAT);//参数nsems,此时为中间值1，指定信号灯集包含信号灯的数目
    //0666|IPC_CREAT用来表示对信号灯的读写权限
    /*
    从左向右:
    第一位:0表示这是一个8进制数
    第二位:当前用户的经权限:6=110(二进制),每一位分别对就 可读,可写,可执行,6说明当前用户可读可写不可执行
    第三位:group组用户,6的意义同上
    第四位:其它用户,每一位的意义同上,0表示不可读不可写也不可执行
    */
    if(semid==-1)
        printf("creat sem is fail\n");
    //创建共享内存
    shmid=shmget(shmkey,1024,0666|IPC_CREAT);//对共享内存
    if(shmid==-1)
        printf("creat shm is fail\n");

    /*设置信号量的初始值，就是资源个数*/
    union semun{
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    }sem_u;

    sem_u.val=1;    /*设置变量值*/
    semctl(semid,0,SETVAL,sem_u);   //初始化信号量，设置第0个信号量，p()操作为非阻塞的

    /*将共享内存映射到当前进程的地址中，之后直接对进程中的地址addr操作就是对共享内存操作*/

    struct People *addr;
    addr=(struct People*)shmat(shmid,0,0);  //将共享内存映射到调用此函数的内存段
    if(addr==(struct People*)-1)
        printf("shm shmat is fail\n");

    /*向共享内存写入数据*/
    p();
    strcpy((*addr).name,"xiaoming");
/*注意：①此处只能给指针指向的地址直接赋值，不能在定义一个  struct People people_1;addr=&people_1;因为addr在addr=(struct People*)shmat(shmid,0,0);时,已经由系统自动分配了一个地址，这个地址与共享内存相关联，所以不能改变这个指针的指向，否则他将不指向共享内存，无法完成通信了。
注意：②给字符数组赋值的方法。刚才太虎了。。*/
    (*addr).age=10;
    v();

    /*将共享内存与当前进程断开*/
    if(shmdt(addr)==-1)
        printf("shmdt is fail\n");  
}
