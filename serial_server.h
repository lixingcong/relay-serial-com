#ifndef _SERIAL_SERVER_H
#define _SERIAL_SERVER_H

#include <libserialport.h>
#include "utils.h"

typedef struct com_port{
	int fd;
	struct sp_port *port;
	struct sp_port_config *conf;
}com_port_t;

com_port_t *open_com(char *devicename);
void close_com(struct sp_port *port_blue,struct sp_port_config *port_blue_config);


#endif
