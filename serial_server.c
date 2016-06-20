//Time-stamp: < serial_server.c 2016-06-20 23:31:00 >
/*说明：串口端的接收数据，模拟串口
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_BUFFER_LENTH 65530
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
	unsigned short index;
	unsigned short data_size;
	int sockfd;						/* sockdet文件描述符 */
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
	
#ifdef SERIAL_MAIN
int main(int argc,char *argv[]){
	char in[MAX_BUFFER_LENTH];
	char *IP,*PORT;
	/* 用户态的数据包格式ASCII */
	user_content_t *my_content=NULL;
	int sockfd_to_remote;
	
	/* if (argc != 3) { */
	/* 	fprintf(stderr,"usage: %s ip port\n",argv[0]); */
	/* 	return 1; */
	/* } */
	
	/* IP=argv[1]; */
	/* PORT=argv[2]; */
	
	printf("hello\n");

	if(read_input(in));
	my_content=new_user_content_from_str(in);

	/* 建立一個 socket： */
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	/* connect! */
	if(status=(connect(sockfd, res->ai_addr, res->ai_addrlen))<0){
		perror("connect error");
		return 1;
	}
	return 0;
}
#endif
