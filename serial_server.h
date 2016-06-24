#ifndef _SERIAL_SERVER_H
#define _SERIAL_SERVER_H

#include <libserialport.h>
#include "utils.h"

com_port_t *open_com(char *devicename);
void close_com(struct sp_port *port_blue,struct sp_port_config *port_blue_config);


#endif
