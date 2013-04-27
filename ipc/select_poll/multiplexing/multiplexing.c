#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 1234    // the port users will be connecting to

#define BACKLOG 5     // how many pending connections queue will hold

#define BUF_SIZE 200

int fd_A[BACKLOG];    // accepted connection fd
int conn_amount;    // current connection amount

//打印号码和对应的socket id
void showclient()
{
    int i;
    printf("client amount: %d\n", conn_amount);
    for (i = 0; i < BACKLOG; i++) {
        printf("[%d]:%d  ", i, fd_A[i]);
    }
    printf("\n\n");
}

int main(void)
{
    int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in server_addr;    // server address information
    struct sockaddr_in client_addr; // connector's address information
    socklen_t sin_size;
    int yes = 1;
    char buf[BUF_SIZE];
    int ret;
    int i;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    //the SO_REUSEADDR flag tells the kernel to reuse a local socket in TIME_WAIT state, without waiting for its natural timeout to expire.
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;         // host byte order
    server_addr.sin_port = htons(MYPORT);     // short, network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    //INADDR_ANY就是指定地址为0.0.0.0的地址,这个地址事实上表示不确定地址,或“所有地址”、“任意地址”。也就是表示机器上的所用网络地址,比如你的机器上有两张网卡，那么到达这两张网卡的数据，你的socket都可以得到通知
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }
//BACKLOG:The backlog argument defines the maximum length to which the queue of pending(等待的) connections for sockfd  may  grow.  If  a  connection  request arrives when the queue is full, the client may receive an error with an indication of ECONNREFUSED or, if the underlying protocol supports retransmission, the request may be ignored so that a  later reattempt at connection succeeds.
    if (listen(sock_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("listen port %d\n", MYPORT);

    fd_set fdsr;
    int maxsock;
    struct timeval tv;

    conn_amount = 0;
    sin_size = sizeof(client_addr);
    maxsock = sock_fd;

    //为了验证最初fd_A中全部为0
    //for (i = 0; i < BACKLOG; i++) {
    //    printf("%d\n",fd_A[i]);
    //}

    while (1) {
        // initialize file descriptor set
        FD_ZERO(&fdsr);
        FD_SET(sock_fd, &fdsr);

        // timeout setting
        tv.tv_sec = 30;
        tv.tv_usec = 0;

        // add active connection to fd set
        for (i = 0; i < BACKLOG; i++) {
            if (fd_A[i] != 0) {
                FD_SET(fd_A[i], &fdsr);
            }
        }

        ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);//当调用select()时，由内核根据io状态修改fd_set的内容.
        if (ret < 0) {
            perror("select");
            break;
        } else if (ret == 0) {
            printf("timeout\n");
            continue;
        }

        // check every fd in the set
        for (i = 0; i < conn_amount; i++) {
            if (FD_ISSET(fd_A[i], &fdsr)) {
                ret = recv(fd_A[i], buf, sizeof(buf), 0);
                if (ret <= 0) {        // client close
                    printf("client[%d] close\n", i);
                    close(fd_A[i]);
                    FD_CLR(fd_A[i], &fdsr);
                    fd_A[i] = 0;
                } else {        // receive data
                    if (ret < BUF_SIZE)
                        memset(&buf[ret], '\0', 1);
                    printf("client[%d] send:%s\n", i, buf);
                }
            }
        }

        // check whether a new connection comes
        if (FD_ISSET(sock_fd, &fdsr)) {
            //The  accept() system call is used with connection-based socket types (SOCK_STREAM, SOCK_SEQPACKET).  It extracts the first connection request on the queue of pending connections for the listening socket, sockfd, creates a new connected  socket,  and returns a new file descriptor referring to that socket.  The newly created socket is not in the listening state.  The original socket sockfd is unaffected by this call.
            new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
            if (new_fd <= 0) {
                perror("accept");
                continue;
            }

            // add to fd queue
            if (conn_amount < BACKLOG) {
                fd_A[conn_amount++] = new_fd;
                printf("new connection client[%d] %s:%d\n", conn_amount,
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                if (new_fd > maxsock)
                    maxsock = new_fd;
            }
            else {
                printf("max connections arrive, exit\n");
                send(new_fd, "bye", 4, 0);
                close(new_fd);
                break;
            }
        }
        showclient();//打印号码和对应的socket id
    }

    // close other connections
    for (i = 0; i < BACKLOG; i++) {
        if (fd_A[i] != 0) {
            close(fd_A[i]);
        }
    }

    exit(0);
}
