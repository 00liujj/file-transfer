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
 
#define BUF_SIZE (64*1024)


void
file_client(const char *ip, int port, const char *src, const char* dst)
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
    }
    
    printf("connect success!\n");
    void src_func(int skfd, const char* src, const char* dst);
    src_func(skfd, src, dst);
}

struct file_meta {
    int size;
    char src[256];
    char dst[256];
};

void src_func(int skfd, const char* src, const char* dst) {

    FILE *fp = fopen(src, "r");
    if( fp == NULL ) {
        perror("fopen");
        close(skfd);
        return;
    }
 
    fseek(fp, 0, SEEK_END);
    int32_t fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
 
    /// write file meta
    file_meta meta;
    meta.size = fileSize;
    strcpy(meta.src, src);
    strcpy(meta.dst, dst);
    int nwrite = write(skfd, (unsigned char *)&meta, sizeof(meta));
    if(nwrite != sizeof(meta)) {
        perror("write");
        close(skfd);
        exit(1);
    }

    /// read file meta ack
    char metaAck[100];
    int nread = read(skfd, metaAck, sizeof(metaAck));
    if( nread != sizeof(metaAck)) {
        perror("read");
        close(skfd);
        exit(1);
    }
    printf("%s\n", metaAck);
 

    /// write file content
    unsigned char fileBuf[BUF_SIZE];
    int count = 0;
    while(1) {
        int size = fread(fileBuf, 1, BUF_SIZE, fp);
        if (size <= 0) {
            break;
        }
        int size2 = 0;
        while( size2 < size ) {
            int nwrite = write(skfd, fileBuf + size2, size - size2);
            count += nwrite;
            printf("client send %d bytes\n", count);
            if (nwrite < 0) {
                perror("write");
                close(skfd);
                exit(1);
            }
            size2 += nwrite;
        }
    }
    printf("write finish\n");

    /// read content ack
    char contentAck[100];
    nread = read(skfd, contentAck, sizeof(contentAck));
    if( nread != sizeof(contentAck)) {
        perror("read");
        close(skfd);
        exit(1);
    }
    printf("%s\n", contentAck);
 
    fclose(fp);
    close(skfd);
}
 
int
main(int argc, char **argv)
{
    if( argc < 5 ) {
        printf("file client: argument error!\n");
        printf("file_client ip port src dst\n");
        return -1;
    }
 
    int port = atoi(argv[2]);
    file_client(argv[1], port, argv[3], argv[4]);

    return 0;
}
