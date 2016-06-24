#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

/* 创建一个socket，蓝牙接口 */
int create_bluetooth_socket();

#endif
