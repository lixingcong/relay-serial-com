#LDFLAGS=-lserialport

LDFLAGS=
CFLAGS=
DEFINEFLAGS=

# serial -----> main ------> phone
ifeq ($(compile),serial)
	DEFINEFLAGS=-DSERIAL_MAIN
# serial <----- main <------ phone
else ifeq ($(compile),main)
	DEFINEFLAGS=-DSERVER_MAIN
endif

OBJS+=main.o
OBJS+=serial_server.o
OBJS+=phone.o
OBJS+=utils.o

all: $(OBJS)
	$(CC) -o main main.o utils.o $(LDFLAGS)
	$(CC) -o phone phone.o utils.o $(LDFLAGS)
	$(CC) -o serial_server serial_server.o utils.o $(LDFLAGS)

$(OBJS):%.o: %.c
	$(CC) -c $< -o $*.o $(LDFLAGS) $(DEFINEFLAGS)

.PHONY: clean run

clean:
	rm -rf $(OBJS)
	rm -rf main

run:all
	./main
