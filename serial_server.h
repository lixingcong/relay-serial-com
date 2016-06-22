#ifndef _SERIAL_SERVER_H
#define _SERIAL_SERVER_H

#include <libserialport.h>
#include "utils.h"

int open_com(char *devicename, int *fd,struct sp_port *port_blue,struct sp_port_config *port_blue_config);
void close_com(struct sp_port *port_blue,struct sp_port_config *port_blue_config);


#endif
