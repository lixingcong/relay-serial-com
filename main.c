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
#include "serial_server.h"

#define TRUE   1
#define FALSE  0
#define MAXLEN 65500			/* 实际上是65535，留余量 */
#define MAXCLIENTS 5


int main(int argc, char *argv[]) {
	int opt = TRUE;
	int master_socket, addrlen, new_socket, client_socket[MAXCLIENTS], max_clients = MAXCLIENTS,
		activity, i, valread, sd;
	user_content_t *my_contents[MAXCLIENTS+1];
	int max_sd;
	
	char com_devicename[]="/dev/ttyUSB0"; /* 固定的linux串口设备文件 */
	com_port_t *my_com_conf;/* 串口配置 */
	char buffer_com[MAXLEN];	/* 串口缓冲区 */
	char *buffer_com_p=buffer_com;
	char *header=NULL;			/* 封包的header */
	char itoa_buffer[8];
	int buffer_com_data_size=0;
	
	struct addrinfo hints, *res; /* 连接到target的用到的 */
	struct sockaddr_in address;

	struct timeval tv;			/* select超时 */
	
	/* 每个客户端的缓冲区和索引 */
	char buffer[MAXCLIENTS][MAXLEN];
	char *buffer_p[MAXCLIENTS];  //data buffer of 1K
	int buffer_data_size[MAXCLIENTS];
	
	//set of socket descriptors
	fd_set readfds;

	char *PORT;

	if (argc != 2) {
		fprintf(stderr,"usage: %s listen-port\n",argv[0]);
		return 1;
	}
	
	PORT=argv[1];

	/* 打开串口，须root权限 */
	if(NULL==(my_com_conf=open_com(com_devicename))){
		printf("error open com!\n");
		return 1;
	}

	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++) {
		client_socket[i] = 0;
	}

	if((master_socket=create_server_socket("0.0.0.0",PORT))<0){
		printf("error create socket fd\n");
		return 1;
	}

	//set master socket to allow multiple connections , this is just a good habit, it will work without this
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
				   sizeof(opt)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Listening on port %s \n", PORT);
	
	//accept the incoming connection
	addrlen = sizeof(address);
	puts("Waiting for connections ...");

	while (TRUE) {
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(master_socket, &readfds);
		FD_SET(my_com_conf->fd,&readfds);
		
		max_sd = master_socket>my_com_conf->fd?master_socket:my_com_conf->fd;

		//add child sockets to set
		for (i = 0; i < max_clients; i++) {
			//socket descriptor
			sd = client_socket[i];

			//if valid socket descriptor then add to read list
			if (sd > 0)
				FD_SET(sd, &readfds);

			//highest file descriptor number, need it for the select function
			if (sd > max_sd)
				max_sd = sd;
		}

		//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR)) {
			printf("select error");
		}

		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket,
									 (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			printf(
				"New connection , socket fd is %d , ip is : %s , port : %d \n",
				new_socket, inet_ntoa(address.sin_addr),
				ntohs(address.sin_port));


			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++) {
				//if position is empty, create new one
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					// 初始化buffer
					buffer_p[i]=buffer[i];
					memset(buffer_p[i],0,MAXLEN);
					buffer_data_size[i]=0;
					
					printf("Adding to list of sockets as %d\n", i);
					break;
				}
			}
		}

		// 串口读
		if (FD_ISSET(my_com_conf->fd, &readfds)) {
			/* 非阻塞读取 */
			valread=sp_nonblocking_read(my_com_conf->port,buffer_com_p+buffer_com_data_size,MAXLEN);
			if(valread<0){
				printf("read data from com error: %d\n",valread);
				return 1;
				buffer_com_data_size=0;
				buffer_com_p=buffer_com;
			}else{
				buffer_com_data_size+=valread;
				/* 读完所有数据，串口数据包必须以\r\n结尾 */
				if(buffer_com[buffer_com_data_size-2]==13 && buffer_com[buffer_com_data_size-1]==10){
					printf("- - - - - - - - - -\nread from COM ok\n");
					buffer_com_p[buffer_com_data_size]=0;
					/* create relay struct: from serial, to ip */
					my_contents[MAXCLIENTS]=new_user_content_from_str(buffer_com,com_devicename,DIR_TO_PHONE);
					if(!my_contents[MAXCLIENTS]){
						printf("invalid packet!\n");
					}else{
						// 输入合法性只能在打开远程ip时候判断！
						printf("  %s    ---->  %s:%s\n",my_contents[MAXCLIENTS]->data,my_contents[MAXCLIENTS]->ip,my_contents[MAXCLIENTS]->port);
						/* now relay to target : to LAN ip */
						memset(&hints, 0, sizeof hints);
						hints.ai_family = AF_UNSPEC; // AF_INET 或 AF_INET6 可以指定版本
						hints.ai_socktype = SOCK_STREAM;
						hints.ai_flags = AI_PASSIVE; // fill in my IP for me

						if (getaddrinfo(my_contents[MAXCLIENTS]->ip, my_contents[MAXCLIENTS]->port, &hints, &res) != 0) {
							printf("getaddrinfo error!\n");
							/* return 1; */
						}else{
							if((new_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol))>0){
								/* connect! */
								if(connect(new_socket, res->ai_addr, res->ai_addrlen)<0){
									/* 这里应该返回结果 告诉来源：目标拒绝连接 */
									perror("connect error");
								}else{
									if(0==sendall(new_socket,my_contents[MAXCLIENTS]))
										printf("    tcp relay ok!\n");
									else
										printf("    sendall fail.\n");
								}
								close(new_socket);
							}
							
						}
						my_free(my_contents[MAXCLIENTS]->ip);
						my_free(my_contents[MAXCLIENTS]->port);
						my_free(my_contents[MAXCLIENTS]->data);
						my_free(my_contents[MAXCLIENTS]);
					}
					/* reset buffer offset */
					buffer_com_data_size=0;
					buffer_com_p=buffer_com;					
				}
			}
		}
		
		//else its some IO operation on some other socket :)
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds)) {
				//Check if it was for closing , and also read the incoming message
				if ((valread = read(sd, buffer_p[i]+buffer_data_size[i],MAXLEN)) == 0) {
					//Somebody disconnected , get his details and print
					buffer[i][buffer_data_size[i]]=0;
					getpeername(sd, (struct sockaddr*) &address,
								(socklen_t*) &addrlen);
					printf("- - - - - - - - - -\nrecv %d bytes form LAN client\n",buffer_data_size[i]);

					//Close the socket and mark as 0 in list for reuse
					close(sd);
					client_socket[i] = 0;

					/* convert port(interger) to char* */
					sprintf(itoa_buffer,"%d",ntohs(address.sin_port));
					header=get_header_ipv4(inet_ntoa(address.sin_addr), itoa_buffer);
					/* create relay struct: from LAN ip, to serial */
					my_contents[i]=new_user_content_from_str(buffer[i],header,DIR_TO_SERIAL);
					my_free(header);
					
					if(!my_contents[i]){
						printf("invalid packet!\n");					
					}else{
						// 输入合法性只能在打开串口时候判断！
						// 如果用户输入/etc/ttyUSB0:xxx还是合法的，但无法打开设备
						if(strcmp(my_contents[i]->device,com_devicename)!=0){
							printf("not such COM device: %s\n",my_contents[i]->device);
						}else{
							printf("  %s    ----> %s\n",my_contents[i]->data,my_contents[i]->device);
							if(sp_blocking_write(my_com_conf->port,my_contents[i]->data,my_contents[i]->data_size,500)<0)
								printf("    write to %s fail!\n",my_contents[i]->device);
							else
								printf("    write COM ok!\n");
						}
						/* now relay to serial */
						my_free(my_contents[i]->data);
						my_free(my_contents[i]->device);
						my_free(my_contents[i]);
					}
						
				}else {
					/* 累加数据 */
					buffer_data_size[i]+=valread;
				}
			}
		}
	}

	sp_free_port(my_com_conf->port);
	sp_free_config(my_com_conf->conf);
	my_free(my_com_conf);
	
	printf("exit..\n");
	return 0;
}

