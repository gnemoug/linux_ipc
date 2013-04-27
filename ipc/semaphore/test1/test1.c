#include <stdio.h>   
#include <sys/types.h>   
#include <sys/sem.h>   
#include <sys/ipc.h>   
#include <stdlib.h>

void main() 
{   
    key_t unique_key; /* 定义一个IPC关键字*/   
    int id;   
    struct sembuf lock_it;
    int i;  
  
    union semun{
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    }options;

    unique_key = ftok(".", 'a'); /* 生成关键字，字符'a'是一个随机种子*/   
    /* 创建一个新的信号量集合*/   
    id = semget(unique_key, 1, IPC_CREAT | IPC_EXCL | 0666);   //用于判断是否存在此信号量，若存在错误退出
    printf("semaphore id=%d\n", id);   
    options.val = 1; /*设置变量值*/   
    semctl(id, 0,SETVAL,options); /*设置索引0的信号量*/  

    /*打印出信号量的值*/
    i = semctl(id, 0, GETVAL, 0);   //返回信号量的值
    printf("value of semaphore at index 0 is %d\n", i);  
      
    /*下面重新设置信号量*/   
    lock_it.sem_num = 0; /*设置哪个信号量*/   
    lock_it.sem_op = -1; /*定义操作*/   
    lock_it.sem_flg = IPC_NOWAIT; /*操作方式*/   
    if (semop(id, &lock_it, 1) == -1) //p(),申请资源
    {   
        printf("can not lock semaphore.\n");   
        exit(1);   
    }  

    i = semctl(id, 0, GETVAL, 0);   
    printf("value of semaphore at index 0 is %d\n", i);  
      
    /*清除信号量*/   
    semctl(id, 0, IPC_RMID, 0);   
}  
