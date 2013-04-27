#include <stdio.h>  
#include <sys/types.h>  
#include <sys/ipc.h>  
#include <sys/sem.h>  
  
#define SHM_KEY 0x33  
#define SEM_KEY 0x44  
  
union semun {  
    int val;  
    struct semid_ds *buf;  
    unsigned short *array;  
};  
  
int P(int semid)  
{  
    struct sembuf sb;  
    sb.sem_num = 0;  
    sb.sem_op = -1;  
    sb.sem_flg = SEM_UNDO;  
      
    if(semop(semid, &sb, 1) == -1) {  
        perror("semop");  
        return -1;  
    }  
    return 0;  
}  
  
int V(int semid)  
{  
    struct sembuf sb;  
    sb.sem_num = 0;  
    sb.sem_op = 1;  
    sb.sem_flg = SEM_UNDO;  
      
    if(semop(semid, &sb, 1) == -1) {  
        perror("semop");  
        return -1;  
    }  
    return 0;  
}  
  
int main(int argc, char **argv)  
{  
    pid_t pid;  
    int i, shmid, semid;  
    int *ptr;  
    union semun semopts;  
  
    /* 创建一块共享内存, 存一个int变量 */  
    if ((shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0600)) == -1) {  
        perror("msgget");  
    }  

    /* 将共享内存映射到进程, fork后子进程可以继承映射 */  
    if ((ptr = (int *)shmat(shmid, NULL, 0)) == (void *)-1) {  
        perror("shmat");  
    }  
    *ptr = 0;  

    /* 创建一个信号量用来同步共享内存的操作 */  
    if ((semid = semget(SEM_KEY, 1, IPC_CREAT | 0600)) == -1) {  
        perror("semget");  
    }  

    /* 初始化信号量 */  
    semopts.val = 1;  
    if (semctl(semid, 0, SETVAL, semopts) < 0) {  
        perror("semctl");  
    }  

    if ((pid = fork()) < 0) {  
        perror("fork");  
    } else if (pid == 0) {      /* Child */  
        /* 子进程对共享内存加1 */  
        for (i = 0; i < 100; i++) {  
            P(semid);  
            (*ptr)++;  
            V(semid);  
            printf("child: %d\n", *ptr);  
        }  
    } else {                    /* Parent */  
        /* 父进程对共享内存减1 */  
        for (i = 0; i < 100; i++) {  
            P(semid);  
            (*ptr)--;  
            V(semid);  
            printf("parent: %d\n", *ptr);  
        }  
        waitpid(pid);  
        sleep(2);
        /* 如果同步成功, 共享内存的值为0 */  
        printf("finally: %d\n", *ptr);  
    }  
  
    return 0;  
}  
