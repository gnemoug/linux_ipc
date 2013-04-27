#include<stdio.h>  
#include<sys/types.h>  
#include<sys/ipc.h>  
#include<sys/sem.h>  
#include<errno.h>  
#include<string.h>  
#include<stdlib.h>  
#include<assert.h>  
#include<time.h>  
#include<unistd.h>  
#include<sys/wait.h>    
#define MAX_SEMAPHORE 10   
#define FILE_NAME "test2.c"  
  
union semun{  
    int val ;  
    struct semid_ds *buf ;  
    unsigned short  *array ;  
    struct seminfo *_buf ;  
}arg;  
struct semid_ds sembuf;  
  
int main()  
{  
    key_t key ;  
    int semid ,ret,i;   
    unsigned short buf[MAX_SEMAPHORE] ;  
    struct sembuf sb[MAX_SEMAPHORE] ;  
    pid_t pid ;  
      
    pid = fork() ;  
    if(pid < 0)  
    {  
        /* Create process Error! */  
        fprintf(stderr,"Create Process Error!:%s\n",strerror(errno));  
        exit(1) ;  
    }     
    if(pid > 0)  
    {  
        /* in parent process !*/          
        key = ftok(FILE_NAME,'a') ;  
        if(key == -1)  
        {  
            /* in parent process*/  
            fprintf(stderr,"Error in ftok:%s!\n",strerror(errno));  
            exit(1) ;     
        }  
  
        semid = semget(key,MAX_SEMAPHORE,IPC_CREAT|0666);  //创建信号量集合
        if(semid == -1)  
        {  
            fprintf(stderr,"Error in semget:%s\n",strerror(errno));   
            exit(1) ;     
        }  
        printf("Semaphore have been initialed successfully in parent process,ID is :%d\n",semid);     
        sleep(2) ;  
        printf("parent wake up....\n");  
        /*父进程在子进程得到semaphore的时候请求semaphore，此时父进程将阻塞直至子进程释放掉semaphore*/  
        /* 此时父进程的阻塞是因为semaphore 1 不能申请，因而导致的进程阻塞*/  
        for(i=0;i<MAX_SEMAPHORE;++i)  
        {  
            sb[i].sem_num = i ;  
            sb[i].sem_op = -1 ;     /*表示申请semaphore*/  
            sb[i].sem_flg = 0 ;  
        }  
        printf("parent is asking for resource...\n");  
        ret = semop(semid , sb ,10); //p() 
        if(ret == 0)  
        {  
            printf("parent got the resource!\n");  
        }         
        /* 父进程等待子进程退出 */  
        waitpid(pid,NULL,0);  
        printf("parent exiting .. \n");  
        exit(0) ;  
    }  
    else  
    {  
        /* in child process! */   
        key = ftok(FILE_NAME,'a') ;  
        if(key == -1)  
        {  
            /* in child process*/  
            fprintf(stderr,"Error in ftok:%s!\n",strerror(errno));  
            exit(1) ;     
        }  
  
        semid = semget(key,MAX_SEMAPHORE,IPC_CREAT|0666);  
        if(semid == -1)  
        {  
            fprintf(stderr,"Error in semget:%s\n",strerror(errno));   
            exit(1) ;     
        }  
        printf("Semaphore have been initialed successfully in child process,ID is:%d\n",semid);   
  
        for(i=0;i<MAX_SEMAPHORE;++i)  
        {  
            /* Initial semaphore */  
            buf[i] = i + 1;  
        }  
        arg.array = buf;  
        ret = semctl(semid , 0, SETALL,arg);  
        if(ret == -1)  
        {  
            fprintf(stderr,"Error in semctl in child:%s!\n",strerror(errno));  
            exit(1) ;  
        }         
        printf("In child , Semaphore Initailed!\n");  
  
        /*子进程在初始化了semaphore之后，就申请获得semaphore*/  
        for(i=0;i<MAX_SEMAPHORE;++i)  
        {
            sb[i].sem_num = i ;  
            sb[i].sem_op = -1 ;  
            sb[i].sem_flg = 0 ;  
        }
        ret = semop(semid , sb , 10);//信号量0被阻塞
        if( ret == -1 )  
        {  
            fprintf(stderr,"子进程申请semaphore失败：%s\n",strerror(errno));  
            exit(1) ;         
        }         
        printf("child got semaphore,and start to sleep 3 seconds!\n");  
        sleep(3) ;    
        printf("child wake up .\n");  
        for(i=0;i < MAX_SEMAPHORE;++i)  
        {  
            sb[i].sem_num = i ;  
            sb[i].sem_op =  +1 ;  
            sb[i].sem_flg = 0 ;       
        }  
        printf("child start to release the resource...\n");  
        ret = semop(semid, sb ,10) ;      
        if(ret == -1)  
        {  
            fprintf(stderr,"子进程释放semaphore失败:%s\n",strerror(errno));  
            exit(1) ;  
        }         
        ret = semctl(semid ,0 ,IPC_RMID);  
        if(ret == -1)  
        {  
            fprintf(stderr,"semaphore删除失败:%s！\n",strerror(errno));  
            exit(1) ;     
        }         
        printf("child exiting successfully!\n");      
        exit(0) ;         
    }  
    return 0;  
}  
