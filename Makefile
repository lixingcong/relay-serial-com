LDFLAGS=-lserialport
CFLAGS=
DEFINEFLAGS=

# serial -----> main ------> phone
ifeq ($(compile),serial)
	DEFINEFLAGS+=-DSERIAL_MAIN
# serial <----- main <------ phone
else ifeq ($(compile),main)
	DEFINEFLAGS+=-DSERVER_MAIN
endif

OBJS+=main.o
OBJS+=serial_server.o
#OBJS+=phone.o
OBJS+=utils.o

all: $(OBJS)
	$(CC) -o main main.o utils.o serial_server.o $(LDFLAGS)
	$(CC) -c phone.c -DSERVER_MAIN 
	$(CC) -o phone_send phone.o utils.o
	$(CC) -c phone.c -DSERIAL_MAIN 
	$(CC) -o phone_recv phone.o utils.o
#	$(CC) -o serial_server serial_server.o utils.o $(LDFLAGS)
#	$(CC) -o main main.o utils.o $(LDFLAGS)

$(OBJS):%.o: %.c
	$(CC) -c $< -o $*.o $(LDFLAGS) $(DEFINEFLAGS)

.PHONY: clean run

clean:
	rm -rf $(OBJS) phone.o
	rm -rf main phone phone_recv phone_send serial_server

run:all
	./main
