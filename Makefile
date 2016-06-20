#LDFLAGS=-lserialport
LDFLAGS=
CFLAGS=
# OBJS+=serial.o
OBJS+=main.o
OBJS+=serial_server.c

all: $(OBJS)
	$(CC) -o main $^ $(LDFLAGS)

$(OBJS):%.o: %.c
	$(CC) -c $< -o $*.o $(LDFLAGS)

.PHONY: clean run

clean:
	rm -rf $(OBJS)
	rm -rf main

run:all
	./main
