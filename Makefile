CC := clang
CFLAGS := -g

all: ui file_system server client

clean:
	rm -rf ui file_system server client main

ui: ui.c 
	$(CC) $(CFLAGS) -o ui ui.c -lncurses -lform -lpthread

file_system: file_system.c 
	$(CC) $(CFLAGS) -o file_system file_system.c -fsanitize=address

server: server.c message.h message.c socket.h user_info_utils.h user_info_utils.c file_system.h file_system.c file.h status_codes.h constants.h 
	$(CC) $(CFLAGS) -o server server.c message.c user_info_utils.c file_system.c -lpthread -fsanitize=address

client: client.c message.h message.c query_util.h query_util.c file_system.h file_system.h file.h status_codes.h constants.h 
	$(CC) $(CFLAGS) -o client client.c message.c query_util.c file_system.c -lpthread -fsanitize=address

main: main.c 
	$(CC) $(CFLAGS) -o main main.c -lncurses -lform -lpthread -fsanitize=address

