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


#ifdef SERIAL_MAIN

int main(int argc,char *argv[])
{
	fd_set readfds;
	int master_socket,new_socket,max_fd; /* 文件描述符 */
	char buffer[MAXLEN];
	int total_recv_bytes=0,valread; /* 接收到的字节 */
	int client_num_ctr=0;
	struct sockaddr_in address;	  /* accept用到 */
	int addrlen=sizeof(address);				  /* address结构体的长度 */
	char *IP,*PORT;

	if (argc != 3) {
		fprintf(stderr,"usage: %s ip port\n",argv[0]);
		return 1;
	}
	
	IP=argv[1];
	PORT=argv[2];

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
			
		}
		/* data come from clients */
		if(new_socket>0 && FD_ISSET(new_socket,&readfds)){
			if ((valread = read(new_socket, buffer,MAXLEN)) == 0) {
				printf("\n#%d client diconnected!\n",++client_num_ctr);
				close(new_socket);
				new_socket=-1;
				total_recv_bytes=0;
			}else{
				total_recv_bytes+=valread;
				buffer[valread] = '\0';
				printf("%s",buffer);
			}
		}
		
	}

	return 0;
}


#endif
