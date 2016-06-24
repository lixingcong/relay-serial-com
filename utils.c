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

#include <libserialport.h>
#include "utils.h"
#include "bluetooth.h"

#define ERROR printf
#define LOGE printf

/* 申请内存 */
void *my_malloc(size_t size) {
	void *tmp = malloc(size);
	if (tmp == NULL)
		exit (EXIT_FAILURE);
	return tmp;
}

/* 输入192.168.4.1:3333:xxxx 
   输入/dev/ttyUSB0:xxxx
   返回一个user_content的指针，使用后记得释放内存 
*/
user_content_t *new_user_content_from_str(char *in,char *header,int direction){
	char *pch;
	int offset[2];
	int occurs=0;
	int header_len=strlen(header);
	user_content_t *tmp;

	if(direction==DIR_TO_PHONE){
		pch=strchr(in,':');
		while(pch!=NULL){
			offset[occurs]=pch-in;
			if((++occurs)==2)break;
			pch=strchr(pch+1,':');
		}
		if(occurs!=2)
			return NULL;

		printf("offset: %d %d\n",offset[0],offset[1]);

		/* 记得释放内存 */
		tmp=my_malloc(sizeof(user_content_t));
		tmp->data_size=(strlen(in)-offset[1]-1+header_len+2);//结束符'\0'和':'
		tmp->index=0;
		tmp->ip=my_malloc(sizeof(char)*(offset[0]+1));// 结束符'\0'
		tmp->port=my_malloc(sizeof(char)*(offset[1]-offset[0]));// 结束符'\0'
		tmp->data=my_malloc(sizeof(char)*tmp->data_size);

		memcpy(tmp->ip,in,offset[0]);
		*(tmp->ip+offset[0])=0;

		memcpy(tmp->port,in+offset[0]+1,offset[1]-offset[0]-1);
		*(tmp->port+offset[1]-offset[0]-1)=0;

		memcpy(tmp->data,header,header_len);
		memcpy(tmp->data+header_len,in+offset[1],tmp->data_size-header_len);
		
		tmp->direction=direction;
	}else if(direction==DIR_TO_SERIAL){
		pch=strchr(in,':');
		if(pch!=NULL)
			offset[occurs]=pch-in;
		else
			return NULL;

		printf("offest[0] is %d\n",offset[0]);
		
		/* 记得释放内存 */
		tmp=my_malloc(sizeof(user_content_t));
		tmp->data_size=(strlen(in)-offset[0]-1+header_len+2);//额外结束符'\0'和':'
		tmp->index=0;

		tmp->device=my_malloc(sizeof(char)*(offset[0]+1));
		tmp->data=my_malloc(sizeof(char)*tmp->data_size);

		memcpy(tmp->device,in,offset[0]);
		*(tmp->device+offset[0])=0;

		memcpy(tmp->data,header,header_len);
		memcpy(tmp->data+header_len,in+offset[0],tmp->data_size-header_len);

		tmp->direction=direction;
		
	}else if(direction==DIR_TO_SERVER){// to server
		// 输入合法性只能在服务端判断！
		/* 记得释放内存 */
		tmp=my_malloc(sizeof(user_content_t));
		tmp->data_size=(strlen(in));
		tmp->index=0;

		tmp->data=my_malloc(sizeof(char)*tmp->data_size);

		memcpy(tmp->data,in,tmp->data_size);
		*(tmp->data+tmp->data_size)=0;
		
		tmp->direction=direction;
	}else if(direction==DIR_TO_BLUETOOTH){
		pch=strchr(in,']');
		offset[0]=pch-in;

		if(offset[0]!=18){
			printf("error MAC format in new_content_from_str!\n");
			return NULL;
		}

		tmp=my_malloc(sizeof(user_content_t));
		tmp->data_size=(strlen(in)-offset[0]-1-1+header_len+2);//减去一对括号，额外包含一个':'和结束符'\0'
		tmp->index=0;

		tmp->data=my_malloc(sizeof(char)*tmp->data_size);

		/*copy MAC address */
		memcpy(tmp->mac,in+1,17);
		*(tmp->mac+18)=0;
		
		memcpy(tmp->data,header,header_len);
		memcpy(tmp->data+header_len,in+offset[0]+1,tmp->data_size-header_len);
		
		tmp->direction=direction;
		
		/* tmp->index=0; */
		return tmp;		
		
	}else{
		printf("error direction\n");
		return NULL;
	}

	return tmp;
}


// TCP bind, lack of listen
int create_server_socket(const char *host,const char *port){
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


/* 将user_content的内容全部发送出去 根据direction 阻塞操作*/
int sendall(user_content_t *in){
	int n,s;
	struct sockaddr_rc addr = { 0 };
	if(in->direction==DIR_TO_SERVER || in->direction==DIR_TO_PHONE){ /* 发送到ip */
		while(in->index < in->data_size) {
			n = send(in->fd, in->data+in->index, in->data_size, 0);
			/* sleep(0.5); */
			if (n == -1) { printf("sendall error!\n");break; }
			in->index += n;
		}
		return n==-1?-1:0; // 失敗時傳回 -1、成功時傳回 0
	}else if(in->direction==DIR_TO_SERIAL){
		if((n=sp_blocking_write(in->com_port,in->data,in->data_size,500))<0){
			printf("error in blocking write to COM!\n");
			return n;
		}
	}else if(in->direction==DIR_TO_BLUETOOTH){
		s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		if(s<0){
			printf("error in socket in sendall\n");
			return -1;
		}
		// set the connection parameters (who to connect to)
		addr.rc_family = AF_BLUETOOTH;
		addr.rc_channel = (uint8_t) 1;
		str2ba(in->mac, &addr.rc_bdaddr );

		if(connect(s, (struct sockaddr *)&addr, sizeof(addr))<0){
			printf("error in connect in sendall\n");
			return -1;
		}
		
		n=write(in->fd, in->data, in->data_size);
		
		return n;
	}else{
		printf("error diretion in sendall!\n");
	}

	return -1;
}

char *get_header_ipv4(char *ip,char *port){
	int len1=strlen(ip);
	int len2=strlen(port);
	char *tmp=my_malloc(sizeof(char)*(len1+len2+2));
	memcpy(tmp,ip,len1);
	*(tmp+len1)=':';
	memcpy(tmp+len1+1,port,len2);
	*(tmp+len1+len2+1)=0;
	return tmp;
}

/* 返回值只能是到串口，到phone，到蓝牙 */
int get_direction(char *in){
	if(in[0]>'0'&&in[0]<'9')
		return DIR_TO_PHONE;
	if(in[0]=='[')
		return DIR_TO_BLUETOOTH;
	if(in[0]=='/')
		return DIR_TO_SERIAL;
	return -1;
}

// 转发包，前提是user_content要满足非NULL的条件
int redirect_from_user_content(user_content_t *in){
	struct addrinfo hints, *res; /* 连接到target的用到的 */
	
	switch (in->direction){		
	case DIR_TO_PHONE:
		printf("  %s    ---->  %s:%s\n",in->data,in->ip,in->port);
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC; // AF_INET 或 AF_INET6 可以指定版本
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE; // fill in my IP for me

		if (getaddrinfo(in->ip, in->port, &hints, &res) != 0) {
			printf("getaddrinfo error!\n");
		}else{
			if((in->fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))>0){
				/* connect! */
				if(connect(in->fd, res->ai_addr, res->ai_addrlen)<0){
					/* 这里应该返回结果 告诉来源：目标拒绝连接 */
					perror("connect error");
				}else{
					if(0==sendall(in))
						printf("    tcp relay ok!\n");
					else
						printf("    sendall fail.\n");
				}
				close(in->fd);
			}else{
				perror("socket create error");
			}
		}
		my_free(in->ip);
		my_free(in->port);
		my_free(in->data);
		break;
#ifdef MODULE_BLUETOOTH
	case DIR_TO_BLUETOOTH:
		
		break;
#endif
		
#ifdef MODULE_SERIAL		
	case DIR_TO_SERIAL:
		break;
#endif		
	default:
		printf("error direction!=n");
		return -1;
		break;
	}
}
