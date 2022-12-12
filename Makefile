CC := clang
CFLAGS := -g

all: ui file_system server client

clean:
	rm -rf ui file_system server client

ui: ui.c 
	$(CC) $(CFLAGS) -o ui ui.c -lncurses -lform

file_system: file_system.c 
	$(CC) $(CFLAGS) -o file_system file_system.c -fsanitize=address

server: server.c message.h message.c socket.h
	$(CC) $(CFLAGS) -o server server.c message.c -lpthread

client: client.c message.h message.c
	$(CC) $(CFLAGS) -o client client.c message.c -lpthread
