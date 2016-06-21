#LDFLAGS=-lserialport

LDFLAGS=
CFLAGS=
DEFINEFLAGS=

ifeq ($(compile),serial)
	DEFINEFLAGS=-DSERIAL_MAIN
else ifeq ($(compile),main)
	DEFINEFLAGS=-DSERVER_MAIN
endif

OBJS+=main.o
OBJS+=serial_server.o

all: $(OBJS)
	$(CC) -o main main.o $(LDFLAGS)
	$(CC) -o serial_server serial_server.o $(LDFLAGS)

$(OBJS):%.o: %.c
	$(CC) -c $< -o $*.o $(LDFLAGS) $(DEFINEFLAGS)

.PHONY: clean run

clean:
	rm -rf $(OBJS)
	rm -rf main

run:all
	./main
