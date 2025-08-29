#define _POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 将指定的套接字设置为非阻塞模式，不带延迟标志。 */
int socketSetNonBlockNoDelay(int fd) {
    int flags, yes = 1;
    // 将套接字设置为非阻塞。注意，用于 F_GETFL 和 F_SETFL 的 fcntl(2) 不能被信号中断。
    if ((flags = fcntl(fd, F_GETFL)) == -1) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1 ) return -1;

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    return 0;
}

int createTCPServer(int port) {
    int s, yes = 1;
    struct sockaddr_in sa;
    if((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1 || listen(s, 511) == -1)
    {
        close(s);
        return -1;
    }
    return s;
}

/* 创建一个 TCP 套接字并将其连接到指定地址。
* 成功时返回套接字描述符，否则返回 -1。
*
* 如果 'nonblock' 非零，则套接字将处于非阻塞状态，
* 并且 connect() 尝试也不会阻塞，但套接字
* 可能无法立即准备好写入。*/
int TCPConnect(char *addr, int port, int nonblock) {
    int s, retval = -1;
    struct addrinfo hints, *servinfo, *p;

    char portstr[6];
    snprintf(portstr, sizeof(portstr), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(addr, portstr, &hints, &servinfo) != 0) return -1;

    for(p = servinfo; p != NULL; p = p->ai_next) {
        /* 尝试创建套接字并连接。
        * 如果在 socket() 调用或 connect() 中失败，我们将使用
        * servinfo 中的下一个条目重试。*/
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;

        if (nonblock && socketSetNonBlockNoDelay(s) == -1) {
           close(s);
           break;
        }
    
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
            if(errno == EINPROGRESS && nonblock) return s;
            close(s);
            break;
        }
        retval = s;
        break;
    }
    freeaddrinfo(servinfo);
    return retval;
}

/* 如果监听套接字发出信号，表示有新的连接已准备好接受，
则 accept(2) 该连接，
并在出错时返回 -1，或在成功时返回新的客户端套接字。*/
int acceptClient(int server_socket) {
    int s;
    while(1) {
        struct sockaddr_in sa;
        socklen_t slen = sizeof(sa);
        s = accept(server_socket, (struct sockaddr*)&sa, &slen);
        if (s == -1) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }
        break;
    }
    return s;
}

void *chatMalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        perror("Out of memory");
        exit(1);
    }
    return ptr;
}

void *chatRealloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    if (ptr == NULL) {
        perror("Out of memory");
        exit(1);
    }
    return ptr;
}