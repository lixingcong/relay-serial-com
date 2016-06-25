## 简单的串口转发到内网ip的小程序

## 简介

特点
- 支持三网转发：蓝牙、串口、ip端口相互发送
- 使用select模型进行多路复用
- 基于TCP协议，不需要考虑发包顺序
- 使用autoconf配置，可移植到openwrt
- 对某些高大上的“物联网”项目有一定的参考意义

## 依赖

串口库[libserialport](http://sigrok.org/gitweb/?p=libserialport.git)(已经集成为共享库)

蓝牙库[bluez](http://www.bluez.org/development/git/)

默认打开/dev/ttyUSB0作为串口设备。根据实际情况修改main.c中的串口设备名
注意修改bluetooth.c中的蓝牙MAC设备地址

## 使用

编译

	sh autogen.sh
	
	# 若不想编译模块其中之一，不写它就ok
	./configure --enable-serial=yes --enable-bluetooth=yes
	make

主程序(负责包转发)

	# 监听4000端口
	sudo src/main 4000

运行phone_recv即可收到数据

	# 监听4567端口
	src/phone_recv 127.0.0.1 4567

使用类似HTerm的串口软件测试：
	
	# 从串口发送数据给ip
	# 数据包格式
	127.0.0.1:4567:hello\r\n

运行phone_send即可发送数据给串口

	src/phone_send 127.0.0.1 4000
	# 从局域网ip发送给串口
	/dev/ttyUSB0:hello\n
	
发送指令数据包格式

	# 发给串口:：设备+消息
	/dev/ttyUSB1:hellomessage!
	
	# 发给ip端口：ip+port+消息 
	192.168.5.1:4445:helloworld!
	
	# 发给蓝牙：对方蓝牙地址+消息
	[00:11:22:33:44:55]:hellofrombluetooth!
	
## 蓝牙

使用bluez-tools（在openwrt下叫bluez-utils）开启蓝牙

	# 打开设备
	hciconfig hci0 up
	# 让其可见（实际上只允许通过MAC连接）
	hciconfig hci0 pscan
	
测试

	src/blue_recv
	src/blue_send
	src/serial_recv
	src/serial_send
	
## openwrt

先下载这个[Makefile](https://github.com/lixingcong/relay-serial-com/blob/master/openwrt/Makefile)放入到openwrt/package/relay-serial-com目录下，如果需要定制组件，编辑Makefile的CONFIGURE_ARGS变量。

	# 首先准备好SDK，自行downloads.openwrt.org下载
	cd openwrt
	make menuconfig
	# 选择Network->relay-serial-com
	make package/relay-serial-com/compile V=99

## 鸣谢

本项目参考了shadowsocks-libev的[tunnel.c](https://github.com/shadowsocks/shadowsocks-libev/blob/master/src/tunnel.c)作为tcp转发核心原理，还有[@silv3rm00n](https://gist.github.com/silv3rm00n/5604330)的select服务器例程。其中openwrt的编译脚本参考了[openwrt-shadowsocks](https://github.com/shadowsocks/openwrt-shadowsocks/blob/master/Makefile)的共享库创建方法。
