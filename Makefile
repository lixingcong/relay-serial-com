LDFLAGS=-lserialport -lbluetooth
CFLAGS=
DEFINEFLAGS=


ifeq ($(serial),1)
	DEFINEFLAGS+=-DMODULE_SERIAL
endif 

ifeq ($(bluetooth),1)
	DEFINEFLAGS+=-DMODULE_BLUETOOTH
endif

OBJS+=main.o
OBJS+=serial_server.o
OBJS+=utils.o
OBJS+=bluetooth.o

all: $(OBJS)
	$(CC) -o main main.o utils.o serial_server.o bluetooth.o $(LDFLAGS)

# serial -----> main ------> phone
	$(CC) -c phone.c -DSERVER_MAIN 
	$(CC) -o phone_send phone.o utils.o $(LDFLAGS)

# serial <----- main <------ phone
	$(CC) -c phone.c -DSERIAL_MAIN 
	$(CC) -o phone_recv phone.o utils.o $(LDFLAGS)

#	$(CC) -o blue_recv bluetooth.o utils.o $(LDFLAGS)

$(OBJS):%.o: %.c
	$(CC) -c $< -o $*.o $(LDFLAGS) $(DEFINEFLAGS)
#	$(CC) -c $< -o $*.o $(LDFLAGS) $(DEFINEFLAGS) -DTEST_DIRECTION

.PHONY: clean run

clean:
	rm -rf $(OBJS) phone.o
	rm -rf main phone phone_recv phone_send serial_server blue_recv

run:all
	./main
