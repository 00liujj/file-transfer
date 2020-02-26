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
 
#define BUF_SIZE  (8192)
 
unsigned char fileBuf[BUF_SIZE];
 
/*
 * send file
 */
void
file_server(int port, const char *path)
{
 
    if( !path ) {
        printf("file server: file path error!\n");
        return;
    }

    int skfd, cnfd;
 
    //创建tcp socket
    if((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    } else {
        printf("socket success!\n");
    }
 
    //创建结构  绑定地址端口号
    struct sockaddr_in sockAddr, cltAddr;
    memset(&sockAddr, 0, sizeof(struct sockaddr_in));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(port);
 
    //bind
    if(bind(skfd, (struct sockaddr *)(&sockAddr), sizeof(struct sockaddr)) < 0) {
        perror("Bind");
        exit(1);
    } else {
        printf("bind success!\n");
    }
 
    //listen   监听  最大4个用户
    if(listen(skfd, 4) < 0) {
        perror("Listen");
        exit(1);
    } else {
        printf("listen success!\n");
    }
 
    /* 调用accept,服务器端一直阻塞，直到客户程序与其建立连接成功为止*/
    socklen_t addrLen = sizeof(struct sockaddr_in);
    if((cnfd = accept(skfd, (struct sockaddr *)(&cltAddr), &addrLen)) < 0) {
        perror("Accept");
        exit(1);
    } else {
        printf("accept success!\n");
    }
 

    int fileSize = 0;
    int nread = read(cnfd, (unsigned char *)&fileSize, 4);
    if( nread != 4 ) {
        printf("file size error!\n");
        close(cnfd);
        exit(-1);
    }
    printf("file size:%d\n", fileSize);
 
    int nwrite = write(cnfd, "OK", 2);
    if( nwrite != 2 ) {
        perror("write");
        close(cnfd);
        exit(1);
    }
 
    FILE* fp = fopen(path, "w");
    if( fp == NULL ) {
        perror("fopen");
        close(cnfd);
        close(skfd);
        return;
    }
 
    while(1) {
        int nread = read(cnfd, fileBuf, sizeof(fileBuf));
        printf("server received %d bytes\n", nread);
        if (nread <= 0) {
            break;
        }
        int nwrite = fwrite(fileBuf, 1, nread, fp);
        if (nwrite != nread) {
            perror("fwrite");
            close(cnfd);
            close(skfd);
            return;
        }
    }
    fclose(fp);

    close(cnfd);
    close(skfd);
}
 
int
main(int argc, char **argv)
{
    if( argc < 3 ) {
        printf("file server: argument error!\n");
        printf("file_server port /tmp/temp\n");
        return -1;
    }
 
    int port = atoi(argv[1]);
    file_server(port, argv[2]);
 
    return 0;
}