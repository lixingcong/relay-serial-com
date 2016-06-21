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

#include "utils.h"

#define ERROR printf
#define LOGE printf

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


// TCP bind, lack of listen
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
