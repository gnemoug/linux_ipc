#include <stdio.h>  
#include <sys/types.h>  
#include <sys/ipc.h>  
#include <semaphore.h>  
#include <fcntl.h>           /* For O_* constants */  
#include <sys/stat.h>        /* For mode constants */  
#include <stdlib.h>  

#define SHM_KEY 0x33  

int main(int argc, char **argv)  
{  
    pid_t pid;  
    int i, shmid;  
    int *ptr;  
    sem_t *sem;  

    /* 创建一块共享内存, 存一个int变量 */  
    if ((shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0600)) == -1) {  
        perror("msgget");  
    }  

    /* 将共享内存映射到进程, fork后子进程可以继承映射 */  
    if ((ptr = (int *)shmat(shmid, NULL, 0)) == (void *)-1) {  
        perror("shmat");  
    }  
    *ptr = 0;  

    /* posix的有名信号量是kernel persistent的 
     * 调用sem_unlink删除以前的信号量 */  
    sem_unlink("/mysem");  

    /* 创建新的信号量, 初值为1, sem_open会创建共享内存 
     * 所以信号量是内核持续的 */  
    if ((sem = sem_open("/mysem", O_CREAT, 0600, 1)) == SEM_FAILED) {  
        perror("sem_open");  
    }

    if ((pid = fork()) < 0) {  
        perror("fork");
    } else if (pid == 0) {      /* Child */  
        /* 子进程对共享内存加1 */  
        for (i = 0; i < 100; i++) {  
            sem_wait(sem);  
            (*ptr)++;  
            sem_post(sem);  
            printf("child: %d\n", *ptr);  
        }
    } else {                    /* Parent */  
        /* 父进程对共享内存减1 */  
        for (i = 0; i < 100; i++) {  
            sem_wait(sem);  
            (*ptr)--;  
            sem_post(sem);  
            printf("parent: %d\n", *ptr);  
        }  
        waitpid(pid);  
        /* 如果同步成功, 共享内存的值为0 */  
        printf("finally: %d\n", *ptr);  
        sem_unlink("/mysem");
    }  
  
    return 0;  
}  
