/*client.c:从共享内存中读出People*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semid;
int shmid;

/*信号量的P操作*/
void p()
{
    struct sembuf sem_p;
    sem_p.sem_num=0;
    sem_p.sem_op=-1;
    if(semop(semid,&sem_p,1)==-1)
        printf("p operation is fail\n");  
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
    key_t semkey;
    key_t shmkey;
    semkey=ftok("../test/client/VenusDB.cbp",0);
    shmkey=ftok("../test/client/main.c",0);

    struct People{
        char name[10];
        int age;
    };

    /*读取共享内存和信号量的IPC*/ 
    semid=semget(semkey,0,0666);
    if(semid==-1)
        printf("creat sem is fail\n");
    shmid=shmget(shmkey,0,0666);
    if(shmid==-1)
        printf("creat shm is fail\n");

    /*将共享内存映射到当前进程的地址中，之后直接对进程中的地址addr操作就是对共享内存操作*/
    struct People *addr;
    addr=(struct People*)shmat(shmid,0,0);
    if(addr==(struct People*)-1)
        printf("shm shmat is fail\n");

    /*从共享内存读出数据*/
    p();
    printf("name:%s\n",addr->name);
    printf("age:%d\n",addr->age);
    v();

    /*将共享内存与当前进程断开*/
    if(shmdt(addr)==-1)
        printf("shmdt is fail\n");
 
    /*IPC必须显示删除。否则会一直留存在系统中*/
    if(semctl(semid,0,IPC_RMID,0)==-1)
        printf("semctl delete error\n");
    if(shmctl(shmid,IPC_RMID,NULL)==-1)
        printf("shmctl delete error\n");
}
