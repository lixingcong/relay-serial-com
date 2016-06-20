#LDFLAGS=-lserialport
LDFLAGS=
CFLAGS=
# OBJS+=serial.o
OBJS+=main.o
OBJS+=serial_server.o

all: $(OBJS)
	$(CC) -o main $^ $(LDFLAGS) -DSERVER_MAIN

serial: serial_server.c
	$(CC) -o main $^ $(LDFLAGS) -DSERIAL_MAIN
	
$(OBJS):%.o: %.c
	$(CC) -c $< -o $*.o $(LDFLAGS)

.PHONY: clean run serial 

clean:
	rm -rf $(OBJS)
	rm -rf main

run:all
	./main
