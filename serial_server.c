//Time-stamp: < serial_server.c 2016-06-21 10:34:43 >
/*说明：串口端的接收数据，模拟串口
 */
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


#define MAX_BUFFER_LENTH 65500
#define ERROR printf
#define LOGE printf
/* 释放内存 */
#define my_free(ptr)							\
    do {										\
        free(ptr);								\
        ptr = NULL;								\
    } while(0)

/* 封包用户输入的数据（来自串口），仿照shadowsocks-libev的封装包格式 */
typedef struct user_content {
	int index;
	int data_size;
	int sockfd;						/* sockdet文件描述符 */
	int connected;				/* 是否已经连接，类似shadowsocks */
	char *ip;
	char *port;
	char *data;
} user_content_t;


int read_input(char *in){
	int len;
	scanf("%s",in);
	return strlen(in);
}

// TCP
static int create_server_socket(const char *host,const char *port){
	struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, server_sock;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;               /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM;              /* We want a UDP socket */
    hints.ai_flags    = AI_PASSIVE | AI_ADDRCONFIG; /* For wildcard IP address */
    hints.ai_protocol = IPPROTO_TCP;

    s = getaddrinfo(host, port, &hints, &result);
    if (s != 0) {
        LOGE("[tcp] getaddrinfo: %s", gai_strerror(s));
        return -1;
    }
	rp=result;

	for (/*rp = result*/; rp != NULL; rp = rp->ai_next) {
        server_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_sock == -1) {
            continue;
        }

        int opt = 1;
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		s = bind(server_sock, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        } else {
            ERROR("[tcp] bind");
        }

        close(server_sock);
	}

	if (rp == NULL) {
        LOGE("[udp] cannot bind");
        return -1;
    }
    freeaddrinfo(result);
    return server_sock;
}

/* 申请内存 */
void *my_malloc(size_t size) {
	void *tmp = malloc(size);
	if (tmp == NULL)
		exit (EXIT_FAILURE);
	return tmp;
}

/* 输入192.168.4.1:3333:xxxx 返回一个user_content的指针，使用后记得释放内存 */
user_content_t *new_user_content_from_str(char *in){
	char *pch;
	int offset[2];
	int occurs=0;
	
	pch=strchr(in,':');
	while(pch!=NULL){
		offset[occurs]=pch-in+1;
		if((++occurs)==2)break;
		pch=strchr(pch+1,':');
	}
	if(occurs!=2)
		return NULL;

	/* 记得释放内存 */
	user_content_t *tmp=my_malloc(sizeof(user_content_t));
	tmp->data_size=(strlen(in)-offset[1]);
	tmp->index=0;
	tmp->sockfd=-1;
	tmp->ip=my_malloc(sizeof(char)*(offset[0]-1));
	tmp->port=my_malloc(sizeof(char)*(offset[1]-1));	
	tmp->data=my_malloc(sizeof(char)*tmp->data_size);

	memcpy(tmp->ip,in,offset[0]);
	*(tmp->ip+offset[0]-1)=0;

	memcpy(tmp->port,in+offset[0],offset[1]-offset[0]-1);
	*(tmp->port+offset[1]-1)=0;
	
	memcpy(tmp->data,in+offset[1],tmp->data_size+1);
	*(tmp->data+tmp->data_size)=0;

	return tmp;
}

/* 将user_content的内容全部发送出去 阻塞操作*/
int sendall(int s, user_content_t *in){
	int n;
	while(in->index < in->data_size) {
		n = send(s, in->data+in->index, in->data_size, 0);
		printf("send %d bytes\n, now index is %d",n,in->index);//,in->data);
		/* sleep(0.5); */
		if (n == -1) { printf("sendall error!\n");break; }
		in->index += n;
	}
	return n==-1?-1:0; // 失敗時傳回 -1、成功時傳回 0
}
	
#ifdef SERIAL_MAIN
int main(int argc,char *argv[]){
	char in[MAX_BUFFER_LENTH];
	char *IP,*PORT;
	/* 用户态的数据包格式ASCII */
	user_content_t *my_content=NULL;
	int sockfd_to_remote,filefd,byte_readed;

	int status;
	struct addrinfo hints, *res; /* 连接到remote */
	
	if (argc != 3) {
		fprintf(stderr,"usage: %s ip port\n",argv[0]);
		return 1;
	}
	
	IP=argv[1];
	PORT=argv[2];
	
	printf("hello\n");

	/* if(read_input(in)); */
	/* my_content=new_user_content_from_str(in); */

	filefd=open("dd.txt",O_RDWR);
	byte_readed = read(filefd, in, sizeof(in));
	printf("%d bytes readed\n",byte_readed);
	getchar();
	in[byte_readed]=0;
	
	my_content=my_malloc(sizeof(user_content_t));
	my_content->data_size=strlen(in);
	my_content->index=0;
	my_content->data=in;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET 或 AF_INET6 可以指定版本
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	/* if ((status = getaddrinfo(my_content->ip, my_content->port, &hints, &res)) != 0) { */
	if ((status = getaddrinfo(IP, PORT, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return 1;
	}
	
	/* 建立一個 socket： */
	sockfd_to_remote = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	/* connect! */
	if(status=(connect(sockfd_to_remote, res->ai_addr, res->ai_addrlen))<0){
		perror("connect error");
		return 1;
	}

	printf("hello form \n");

	if(-1==sendall(sockfd_to_remote,my_content))printf("sendall fail.\n");

	
	return 0;
}
#endif
