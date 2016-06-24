# COPYRIGHT Lixingcong
# 2016-06-25
# https://github.com/lixingcong

LDFLAGS=-lserialport -lbluetooth
DEFINEFLAGS=

# run 'make serial=1' to compile a serial relay module
ifeq ($(serial),1)
	DEFINEFLAGS+=-DMODULE_SERIAL
endif 

# run 'make bluetooth=1' to compile a bluetooth relay module
ifeq ($(bluetooth),1)
	DEFINEFLAGS+=-DMODULE_BLUETOOTH
endif

OBJS+=main.o
OBJS+=serial_server.o
OBJS+=utils.o
OBJS+=bluetooth.o

all: $(OBJS)
# main server runs on my ubuntu 16.04
	$(CC) -o main main.o utils.o serial_server.o bluetooth.o $(LDFLAGS)

# serial -----> main ------> phone
	$(CC) -c phone.c -DSERVER_MAIN 
	$(CC) -o phone_send phone.o utils.o serial_server.o bluetooth.o $(LDFLAGS)

# serial <----- main <------ phone
	$(CC) -c phone.c -DSERIAL_MAIN 
	$(CC) -o phone_recv phone.o utils.o serial_server.o bluetooth.o $(LDFLAGS)

# bluetooth-server recv data
	$(CC) -c bluetooth.c -DBLUETOOTH_RECV
	$(CC) -o blue_recv bluetooth.o utils.o serial_server.o $(LDFLAGS)

# bluetooth-client send data
	$(CC) -c bluetooth.c -DBLUETOOTH_SEND
	$(CC) -o blue_send bluetooth.o utils.o serial_server.o $(LDFLAGS)
	rm bluetooth.o

# operate a /dev/ttyUSB0 to send data
	$(CC) -o serial_send serial_server.c utils.o $(LDFLAGS) -DSERIAL_SEND

$(OBJS):%.o: %.c
	$(CC) -c $< -o $*.o $(LDFLAGS) $(DEFINEFLAGS)
# test user-content converter
#	$(CC) -c $< -o $*.o $(LDFLAGS) $(DEFINEFLAGS) -DTEST_DIRECTION

.PHONY: clean run

clean:
	rm -rf $(OBJS) 
	rm -rf phone.o
	rm -rf main phone phone_recv phone_send serial_server serial_send blue_recv blue_send
run:all
	sudo ./main
