//Time-stamp: < serial_server.c 2016-06-25 00:56:27 >
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
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include "serial_server.h"
#include "utils.h"

#ifdef SERIAL_SEND
#define SERIAL_MAIN
#endif

#ifdef SERIAL_RECV
#define SERIAL_MAIN
#endif

/* 待打开的设备名字 */
char device_name1[] = "/dev/ttyUSB0";

user_content_t *open_com(char *devicename) {
	user_content_t *tmp = my_malloc(sizeof(user_content_t));
	/* 打开设备 */
	if (sp_get_port_by_name(devicename, &(tmp->com_port)) != SP_OK) {
		fprintf(stderr, "Cannot find the serial port %s\n", devicename);
		sp_free_port(tmp->com_port);
		my_free(tmp);
		return NULL;
	}

	/* 打开串口 */
	if (sp_open(tmp->com_port, SP_MODE_READ_WRITE) != SP_OK) {
		fprintf(stderr, "Cannot open the serial port %s\n", devicename);
		sp_free_port(tmp->com_port);
		my_free(tmp);
		return NULL;
	}

	/* 文件描述符 */
	if (sp_get_port_handle(tmp->com_port, &(tmp->fd)) != SP_OK) {
		fprintf(stderr, "Cannot get the serial port fd\n");
		sp_free_port(tmp->com_port);
		my_free(tmp);
		return NULL;
	} else {
		printf("open %s ok, the fd is %d\n", devicename, tmp->fd);
	}
	/* 配置结构体 */
	sp_new_config(&(tmp->com_conf));
	sp_set_config_baudrate(tmp->com_conf, 115200);
	sp_set_config_parity(tmp->com_conf, SP_PARITY_NONE);
	sp_set_config_bits(tmp->com_conf, 8);
	sp_set_config_stopbits(tmp->com_conf, 1);
	sp_set_config_flowcontrol(tmp->com_conf, SP_FLOWCONTROL_NONE);

	/* 设置串口 */
	if (sp_set_config(tmp->com_port, tmp->com_conf) != SP_OK) {
		fprintf(stderr, "Cannot configure the serial port\n");
		sp_free_port(tmp->com_port);
		sp_free_config(tmp->com_conf);
		my_free(tmp);
		return NULL;
	}

	return tmp;
}

void close_com(struct sp_port *port_blue,
		struct sp_port_config *port_blue_config) {
	sp_free_port(port_blue);
	sp_free_config(port_blue_config);
}

// 发送到串口
#ifdef SERIAL_MAIN

int main() {
	int i;
	enum sp_return return_value;
	struct sp_port *port_blue;
	struct sp_port_config *port_blue_config;
	char buffer[1024];
	int serialfd; /* 串口文件描述符 */
	fd_set readfds;

	/* 打开设备 */
	if(sp_get_port_by_name(device_name1, &port_blue) != SP_OK) {
		fprintf(stderr, "Cannot find the serial port %s\n",device_name1);
		return 1;
	}

	/* 打开串口 */
	if(sp_open(port_blue, SP_MODE_READ_WRITE) != SP_OK) {
		fprintf(stderr, "Cannot open the serial port %s\n",device_name1);
		return 1;
	}

	if(sp_get_port_handle(port_blue,&serialfd)!=SP_OK) {
		fprintf(stderr, "Cannot get the serial port fd\n");
		return 1;
	} else
	printf("got seriadfd is %d\n",serialfd);
	/* 配置结构体 */
	sp_new_config(&port_blue_config);
	sp_set_config_baudrate(port_blue_config, 115200);
	sp_set_config_parity(port_blue_config, SP_PARITY_NONE);
	sp_set_config_bits(port_blue_config, 8);
	sp_set_config_stopbits(port_blue_config, 1);
	sp_set_config_flowcontrol(port_blue_config, SP_FLOWCONTROL_NONE);

	/* 设置串口 */
	if(sp_set_config(port_blue, port_blue_config) != SP_OK) {
		fprintf(stderr, "Cannot configure the serial port\n");
		return 1;
	}

#ifdef SERIAL_RECV
	printf("waiting for COM data...\n");

	FD_ZERO(&readfds);
	FD_SET(serialfd,&readfds);

	while(1) {
		select(serialfd+1,&readfds,NULL,NULL,NULL);
		if(FD_ISSET(serialfd,&readfds)) {
			i=sp_blocking_read(port_blue,buffer,2000,500);
			if(i<0)printf("read bytes less than zero\n");
			else {
				printf("%s",buffer);
			}
		}
	}

#endif	

	/* 写入 */
#ifdef SERIAL_SEND
	/* 返回值是写入成功的字节数目 */
	while(1) {
		printf("input str to send:\n");
		scanf("%s",buffer);
		i=strlen(buffer);
		/* 使得从终端输入的以\r\n结尾 */
		buffer[i]=13;
		buffer[i+1]=10;
		buffer[i+2]=0;
		return_value=sp_blocking_write(port_blue, buffer,strlen(buffer),400);
		if(return_value<0)
			printf("write to COM error!\n");

	}
#endif	

	/* 释放资源 */
	sp_free_port(port_blue);
	sp_free_config(port_blue_config);
	return 0;
}

#endif

