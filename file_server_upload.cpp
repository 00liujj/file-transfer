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
#include <thread>
 
#define BUF_SIZE  (8192)
 
 
/*
 * send file
 */
void
file_server(int port)
{
    int skfd, cnfd;
 
    //创建tcp socket
    if((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    } else {
        printf("socket success!\n");
    }
 
    //创建结构  绑定地址端口号
    struct sockaddr_in sockAddr;
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


    while (1) {
        /* 调用accept,服务器端一直阻塞，直到客户程序与其建立连接成功为止*/
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(struct sockaddr_in);
        if((cnfd = accept(skfd, (struct sockaddr *)(&clientAddr), &addrLen)) < 0) {
            perror("Accept");
            continue;
        }
        
        printf("accept success!\n");
        void dst_func(int cnfd);
        std::thread t(dst_func, cnfd);
        t.detach();
    }
    close(skfd);
}

struct file_meta {
    int size;
    char src[256];
    char dst[256];
};


void dst_func(int cnfd) {

    /// read file meta
    file_meta meta;
    int nread = read(cnfd, (unsigned char *)&meta, sizeof(meta));
    if( nread != sizeof(meta) ) {
        printf("file size error!\n");
        close(cnfd);
        return;
    }
    printf("size %d, src %s, dst %s\n", meta.size, meta.src, meta.dst);
    
    /// write meta ack
    char metaAck[100] = {"meta ok"};
    int nwrite = write(cnfd, metaAck, sizeof(metaAck));
    if( nwrite != sizeof(metaAck) ) {
        perror("write");
        close(cnfd);
        exit(1);
    }
 
    FILE* fp = fopen(meta.dst, "w");
    if( fp == NULL ) {
        perror("fopen");
        close(cnfd);
        return;
    }
 
    /// read file content
    unsigned char fileBuf[BUF_SIZE];
    int count = 0;
    while(count < meta.size) {
        int nread = read(cnfd, fileBuf, sizeof(fileBuf));
        if (nread <= 0) {
            perror("read");
            break;
        }
        count += nread;
        printf("server received %d bytes\n", count);
        int nwrite = fwrite(fileBuf, 1, nread, fp);
        if (nwrite != nread) {
            perror("fwrite");
            break;
        }
    }
    printf("read finish\n");

    /// write content ack
    char contentAck[100] = {"content ok"};
    nwrite = write(cnfd, contentAck, sizeof(contentAck));
    if( nwrite != sizeof(contentAck) ) {
        perror("write");
        close(cnfd);
        exit(1);
    }

    fclose(fp);
    close(cnfd);
}

 
int
main(int argc, char **argv)
{
    if( argc < 2 ) {
        printf("file server: argument error!\n");
        printf("file_server port\n");
        return -1;
    }

    int port = atoi(argv[1]);
    file_server(port);
 
    return 0;
}
