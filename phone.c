/**
 * 修改版，配合我的client程序使用
 * 使用select进行多路复用
 * https://gist.github.com/silv3rm00n/5604330
 * Handle multiple socket connections with select and fd_set on Linux
 * Silver Moon ( m00n.silv3r@gmail.com)
 */

#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include "utils.h"

#define MAXLEN 65500			/* 实际上是65535，留余量 */




int main(int argc,char *argv[])
{
	user_content_t *my_content=NULL; /* 接收的数据 */
	char buffer[MAXLEN];		/* 接收数据缓冲器 */
	int master_socket,new_socket,max_fd; /* 文件描述符 */
	char *IP,*PORT;				/* socket发送或者监听地址 */
	if (argc != 3) {
		#ifdef SERVER_MAIN
		fprintf(stderr,"usage: %s dst-ip dst-port\n",argv[0]);
		#endif
		#ifdef SERIAL_MAIN
		fprintf(stderr,"usage: %s listen-ip listen-port\n",argv[0]);
		#endif
		return 1;
	}
	
	IP=argv[1];
	PORT=argv[2];

#ifdef SERVER_MAIN
	int filefd,byte_readed;
	struct addrinfo hints, *res; /* 连接到target的用到的 */
	struct sockaddr_in address;


	while(1){
		printf("input a str to send to a device\n");
		/* scanf 默认将空格视为分隔符，不能发送 */
		/* scanf("%s",buffer); */
		fgets(buffer,MAXLEN,stdin);
		my_content=new_user_content_from_str(buffer,"",DIR_TO_SERVER);
		if(my_content==NULL){
			printf("invalid packet!\n");
			continue;
		}
		printf("%s\n",my_content->data);
		
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC; // AF_INET 或 AF_INET6 可以指定版本
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE; // fill in my IP for me
		if (getaddrinfo(IP, PORT, &hints, &res) != 0) {
			printf("getaddrinfo error!\n");
			return 1;
		}
		my_content->fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		/* connect! */
		if(connect(my_content->fd, res->ai_addr, res->ai_addrlen)<0){
			/* 这里应该返回结果 告诉来源：目标拒绝连接 */
			perror("connect error");
		}else{
			if(0==sendall(my_content))
				printf("tcp send ok!\n");
			else
				printf("sendall fail.\n");
		}

		close(my_content->fd);
		my_free(my_content->data);
		my_free(my_content);

	}
	
	return 0;
#endif
#ifdef SERIAL_MAIN
	fd_set readfds;
	int valread; /* 接收到的字节 */
	unsigned int client_num_ctr=0;		/* 客户端counter数 */
	struct sockaddr_in address;	  /* accept用到 */
	int addrlen=sizeof(address);				  /* address结构体的长度 */

	if((master_socket=create_server_socket(IP,PORT))<0){
		printf("error create socket fd\n");
		return 1;
	}
	
    /*  maximum of 1 pending connections*/
	if (listen(master_socket, 1) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	puts("waiting for connections..");

	new_socket=-1;
	while(1){

		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);

		if(new_socket>0){
			FD_SET(new_socket,&readfds);
		}

		if(new_socket>master_socket)
			max_fd=new_socket;
		else
			max_fd=master_socket;
		select(max_fd+1, &readfds, NULL, NULL, NULL);

		/* come from clients,accept it */
		if (FD_ISSET(master_socket, &readfds)){ 
			if((new_socket = accept(master_socket,
									(struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0){
				printf("error accept\n");
				return 1;
			}
			printf(
				"New socket fd is %d, %s:%d \n",
				new_socket, inet_ntoa(address.sin_addr),
				ntohs(address.sin_port));
			
			my_content=my_malloc(sizeof(user_content_t)); /* 接收的数据*/
			my_content->data_size=0;
			my_content->data=my_malloc(sizeof(char)*MAXLEN);			
		}
		/* data come from clients */
		if(new_socket>0 && FD_ISSET(new_socket,&readfds)){
			if ((valread = read(new_socket, buffer,MAXLEN)) == 0) {
				int i;
				for(i=0;i<my_content->data_size;i++)
					printf("%c",*(my_content->data+i));
				printf("\n");
				printf("\n#%d client diconnected! total_recv is %d\n",++client_num_ctr,my_content->data_size);
				close(new_socket);
				my_free(my_content->data);
				my_free(my_content);
				new_socket=-1;
			}else{
				memcpy(my_content->data+my_content->data_size,buffer,valread);
				my_content->data_size+=valread;
			}
		}
		
	}
#endif
	return 0;
}
