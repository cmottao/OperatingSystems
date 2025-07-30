CC=gcc
CFLAGS=-Wall -O2

all: server client create_index fifos

server: server.c utils.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c utils.h
	$(CC) $(CFLAGS) -o client client.c

create_index: create_index.c utils.h
	$(CC) $(CFLAGS) -o create_index create_index.c

fifos:
	@if [ ! -p /tmp/fifo_c2s ]; then mkfifo /tmp/fifo_c2s; fi
	@if [ ! -p /tmp/fifo_s2c ]; then mkfifo /tmp/fifo_s2c; fi

clean:
	rm -f server client create_index
	rm -f /tmp/fifo_c2s /tmp/fifo_s2c 