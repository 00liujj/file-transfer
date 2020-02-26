#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
 
#define BUF_SIZE (8192)
 
unsigned char fileBuf[BUF_SIZE];
 
void
file_client(const char *ip, int port, const char *path)
{
    int skfd;

    //创建tcp socket
    if((skfd=socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket");
        exit(1);
    } else {
        printf("socket success!\n");
    }
 
    //创建结构设定待连接的服务器地址端口号
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ip);
    sockAddr.sin_port = htons(port);
    printf("connecting to server %s, %d\n", ip, port);

    /* 客户端调用connect主动发起连接请求 */
    if(connect(skfd, (struct sockaddr *)(&sockAddr), sizeof(sockAddr)) < 0) {
        perror("ConnectError:");
        exit(1);
    } else {
        printf("connnect success!\n");
    }
 
    FILE *fp = fopen(path, "r");
    if( fp == NULL ) {
        perror("fopen");
        close(skfd);
        return;
    }
 
    fseek(fp, 0, SEEK_END);
    int32_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
 
    int nwrite = write(skfd, (unsigned char *)&fileSize, 4);
    if(nwrite != 4) {
        perror("write");
        close(skfd);
        exit(1);
    }
 
    char buf[10];
    int nread = read(skfd, buf, 2);
    if( nread != 2) {
        perror("read");
        close(skfd);
        exit(1);
    }
    buf[2] = 0;
    printf("client received %s\n", buf);
 
    while(1) {
        int size = fread(fileBuf, 1, BUF_SIZE, fp);
        if (size <= 0) {
            break;
        }
        int size2 = 0;
        while( size2 < size ) {
            int nwrite = write(skfd, fileBuf + size2, size - size2);
            printf("client send %d bytes\n", nwrite);
            if (nwrite < 0) {
                perror("write");
                close(skfd);
                exit(1);
            }
            size2 += nwrite;
        }
    }
 
    fclose(fp);

    close(skfd);
}
 
int
main(int argc, char **argv)
{
    if( argc < 4 ) {
        printf("file client: argument error!\n");
        printf("file_client 192.168.1.10 port /tmp/temp\n");
        return -1;
    }
 
    int port = atoi(argv[2]);
    file_client(argv[1], port, argv[3]);
 
    return 0;
}