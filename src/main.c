/**
 不足之处：valread是公用的，需要修改为局部的变量
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

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "utils.h"
#include "serial_server.h"
#include "bluetooth.h"

#define TRUE   1
#define FALSE  0
#define MAXLEN 65500			/* 实际上是65535，留余量 */
#define MAXCLIENTS 5

int main(int argc, char *argv[]) {
	/* select部份 */
	int opt = TRUE;
	int master_socket, addrlen, new_socket, client_socket[MAXCLIENTS],
			max_clients = MAXCLIENTS, activity, i, valread, sd;
	user_content_t *my_contents[MAXCLIENTS + 1];
	int max_sd;
	struct timeval tv; /* select超时 */
	//set of socket descriptors
	fd_set readfds;

	/* IP部份 */
	struct addrinfo hints, *res; /* 连接到target的用到的 */
	struct sockaddr_in address;
	/* 每个客户端的缓冲区和索引 */
	char buffer[MAXCLIENTS][MAXLEN];
	char *buffer_p[MAXCLIENTS];  //data buffer of 1K
	int buffer_data_size[MAXCLIENTS];
	char itoa_buffer[8]; /* ip地址从网络顺序转成char数组 */
	char *header = NULL; /* 将转换好的ip地址：封包成header */

	/* 串口部份 固定变量 */
	static char com_devicename[] = "/dev/ttyUSB0"; /* 固定的linux串口设备文件 */
	user_content_t *my_com_conf = my_malloc(sizeof(user_content_t));/* 串口配置 */

	/* 串口部份 动态变量 */
#ifdef MODULE_SERIAL
	char buffer_com[MAXLEN]; /* 串口缓冲区 */
	char *buffer_com_p=buffer_com;
	int buffer_com_data_size=0;
#endif

	/* 蓝牙部份 */
#ifdef MODULE_BLUETOOTH
	struct sockaddr_rc blue_rem_addr = {0};
	char blue_buffer[MAXLEN],blue_sender_MAC[18];
	int blue_fd,blue_fd_client,blue_bytes_read;
	socklen_t blue_opt=sizeof(blue_rem_addr);
	user_content_t *blue_user_content;
#endif

	/* args参数 */
	char *PORT;

	if (argc != 2) {
		fprintf(stderr, "usage: %s listen-port\n", argv[0]);
		return 1;
	}

	PORT = argv[1];

#ifdef MODULE_SERIAL
	/* 打开串口，须root权限 */
	if(NULL==(my_com_conf=open_com(com_devicename))) {
		printf("error open com!\n");
		return 1;
	}
#endif

#ifdef MODULE_BLUETOOTH
	/* 打开蓝牙 */
	blue_fd=create_bluetooth_socket();
	if(blue_fd<0) {
		printf("error bluetooth fd is -1\n");
		return -1;
	}
	listen(blue_fd, 1);
#endif

	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++) {
		client_socket[i] = 0;
	}

	if ((master_socket = create_server_socket("0.0.0.0", PORT)) < 0) {
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
		max_sd = master_socket;

#ifdef MODULE_SERIAL
		FD_SET(my_com_conf->fd,&readfds);
		max_sd = max_sd>my_com_conf->fd?max_sd:my_com_conf->fd;
#endif

#ifdef MODULE_BLUETOOTH
		FD_SET(blue_fd,&readfds);
		max_sd = max_sd>blue_fd?max_sd:blue_fd;
#endif

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

			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++) {
				//if position is empty, create new one
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					// 初始化buffer
					buffer_p[i] = buffer[i];
					memset(buffer_p[i], 0, MAXLEN);
					buffer_data_size[i] = 0;

					printf("accepted #%d client\n", i);
					break;
				}
			}
		}

#ifdef MODULE_SERIAL
		// 串口读
		if (FD_ISSET(my_com_conf->fd, &readfds)) {
			/* 非阻塞读取 */
			valread=sp_nonblocking_read(my_com_conf->com_port,buffer_com_p+buffer_com_data_size,MAXLEN);
			if(valread<0) {
				printf("read data from com error: %d\n",valread);
				return 1;
				buffer_com_data_size=0;
				buffer_com_p=buffer_com;
			} else {
				buffer_com_data_size+=valread;
				/* 读完所有数据，串口数据包必须以\r\n结尾 */
				if(buffer_com[buffer_com_data_size-2]==13 && buffer_com[buffer_com_data_size-1]==10) {
					printf("- - - - - - - - - -\nread from COM ok\n");
					buffer_com_p[buffer_com_data_size]=0;

					my_contents[MAXCLIENTS]=new_user_content_from_str(buffer_com,com_devicename,get_direction(buffer_com));
					if(!my_contents[MAXCLIENTS]) {
						printf("invalid packet!\n");
					} else {
						printf("  %s",com_devicename);
						redirect_from_user_content(my_contents[MAXCLIENTS]);
					}
					my_free(my_contents[MAXCLIENTS]);
					/* reset buffer offset */
					buffer_com_data_size=0;
					buffer_com_p=buffer_com;
				}
			}
		}
#endif

#ifdef MODULE_BLUETOOTH
		// 蓝牙读
		if(FD_ISSET(blue_fd,&readfds)) {
			// accept one connection
			blue_fd_client = accept(blue_fd, (struct sockaddr *)&blue_rem_addr, &blue_opt);

			ba2str( &blue_rem_addr.rc_bdaddr, blue_sender_MAC );

			// read data from the client
			blue_bytes_read = read(blue_fd_client, blue_buffer, sizeof(blue_buffer));
			if( blue_bytes_read > 0 ) {
				printf("- - - - - - - - - -\nread from bluetooth ok\n");
				blue_buffer[blue_bytes_read]=0;

				blue_user_content=new_user_content_from_str(blue_buffer,blue_sender_MAC,get_direction(blue_buffer));
				if(!blue_user_content) {
					printf("invalid packet!\n");
				} else {
					printf("  %s",blue_sender_MAC);
					redirect_from_user_content(blue_user_content);
				}
				my_free(blue_user_content);
			} else {
				printf("bluetooth recv data error!\n");
			}

			// close connection
			close(blue_fd_client);
		}
#endif		

		// 局域网ip读
		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];

			if (FD_ISSET(sd, &readfds)) {
				//Check if it was for closing , and also read the incoming message
				if ((valread = read(sd, buffer_p[i] + buffer_data_size[i],
						MAXLEN)) == 0) {
					//Somebody disconnected , get his details and print
					buffer[i][buffer_data_size[i]] = 0;
					getpeername(sd, (struct sockaddr*) &address,
							(socklen_t*) &addrlen);
					printf(
							"- - - - - - - - - -\nread %d bytes from LAN client\n",
							buffer_data_size[i]);

					//Close the socket and mark as 0 in list for reuse
					close(sd);
					client_socket[i] = 0;

					/* convert port(interger) to char* */
					sprintf(itoa_buffer, "%d", ntohs(address.sin_port));
					header = get_header_ipv4(inet_ntoa(address.sin_addr),
							itoa_buffer);
					/* create relay struct: from LAN ip, to serial */
					my_contents[i] = new_user_content_from_str(buffer[i],
							header, get_direction(buffer[i]));

					if (!my_contents[i]) {
						printf("invalid packet!\n");
					} else {
						printf("  %s", header);
						redirect_from_user_content(my_contents[i]);
					}
					my_free(header);
					my_free(my_contents[i]);

				} else {
					/* 累加数据 */
					buffer_data_size[i] += valread;
				}
			}
		}
	}

#ifdef MODULE_SERIAL
	sp_close(my_com_conf->com_port);
	sp_free_port(my_com_conf->com_port);
	sp_free_config(my_com_conf->com_conf);
	my_free(my_com_conf);
#endif

#ifdef MODULE_BLUETOOTH
	close(blue_fd);
#endif

	close(master_socket);
	printf("exit..\n");
	return 0;
}

