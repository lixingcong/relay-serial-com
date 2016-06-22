#ifndef _UTILS_H
#define _UTILS_H


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <string.h>
#include <sys/stat.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

/* 数据流方向 */
#define DIR_TO_PHONE 0
#define DIR_TO_SERIAL 1
#define DIR_TO_SERVER 2

/* 申请内存 */
void *my_malloc(size_t size);
/* 释放内存 */
#define my_free(ptr)							\
    if(ptr)                                     \
    do {										\
        free(ptr);								\
        ptr = NULL;								\
    } while(0)

/* 封包用户输入的数据（来自串口），仿照shadowsocks-libev的封装包格式 */
typedef struct user_content {
	int index;		  /* 已发送字节索引 */
	int data_size;				/* data包的大小 */
	int direction;				/* 发送方向：到ip，到串口，到局域网设备 */
	char *device;				/* 串口 蓝牙 设备名称 */
	char *ip;
	char *port;
	char *data;
} user_content_t;

/* 输入192.168.4.1:3333:xxxxdataxxxx 返回一个user_content的指针，使用后记得释放内存 */
user_content_t *new_user_content_from_str(char *in, char *header, int direction);

// TCP bind, lack of listen
int create_server_socket(const char *host,const char *port);

/* 将user_content的内容全部发送出去 阻塞操作*/
int sendall(int s, user_content_t *in);

#endif
