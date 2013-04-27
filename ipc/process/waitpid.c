#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdio.h>
int main()
{
   pid_t pc,pr;
   pc=fork();
   if (pc<0)/* fork错误*/
   {
       printf("fork error\n");
       exit(1);
   }
   else if(pc==0)/*在子进程中*/
   {
       sleep(10);
       exit(0);
   }
   else
   {
       do {/* 使用了WNOHANG参数，waitpid不会在这里等待 */
           pr=waitpid(pc,NULL,WNOHANG);
           if (pr==0)
           {
               printf("No child exit\n");
               sleep(1);
           }
          }while (pr==0);
       if (pr==pc)
           printf("successfully get child %d\n",pr);
       else
           printf("wait child error\n");
   }
   return 0;
}
