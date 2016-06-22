//Time-stamp: < serial_server.c 2016-06-22 13:56:03 >
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

#include "utils.h"

#define MAX_BUFFER_LENTH 65500
#ifdef SERVER_MAIN
#define SERIAL_MAIN
#endif

int read_input(char *in){
	int len;
	scanf("%s",in);
	return strlen(in);
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

	if(-1==sendall(sockfd_to_remote,my_content))printf("sendall fail.\n");

	//my_free(my_content->data);
	my_free(my_content);
	return 0;
}
#endif
