# 一个简单的串口转发到内网ip的小程序

依赖：libserialport

默认打开/dev/ttyUSB0作为串口设备。根据实际情况修改main.c中的串口设备名

使用方法：

	make compile=main
	# 监听4000端口
	sudo ./main 4000

运行phone_recv即可收到数据

	# 监听4567端口
	./phone_recv 127.0.0.1 4567

使用类似HTerm的串口软件测试：
	
	# 从串口发送数据给ip
	127.0.0.1:4567:hello\r\n

运行phone_send即可发送数据给串口

	./phone_send 127.0.0.1 4000
	# 从局域网ip发送给串口
	/dev/ttyUSB0:hello\n

本项目参考了shadowsocks-libev的[tunnel.c](https://github.com/shadowsocks/shadowsocks-libev/blob/master/src/tunnel.c)作为tcp转发核心原理，还有[@silv3rm00n](https://gist.github.com/silv3rm00n/5604330)的select服务器例程。