#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *finput, *foutput; //FILE表示是文件指针
    char buffer[PIPE_BUF];
    int n;
    finput = popen("echo test!", "r");  //链接到finput的标准输入
    foutput = popen("cat", "w");        //链接到foutput的标准输出
    /*fileno()返回文件描述符，文件描述符是非负整数。打开现存文件或新建文件时，内核会返回一个文件描述符。读写文件也需要使用文件描述符来指定待读写的文件*/
    read(fileno(finput), buffer, strlen("test!"));
    printf("buffer read from finput is %s\n",buffer);
    write(fileno(foutput), buffer, strlen("test"));
    pclose(finput);
    pclose(foutput);
    printf("\n");
    
    exit(EXIT_SUCCESS);
}
