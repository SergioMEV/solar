CC := clang
CFLAGS := -g

all: server client

clean:
	rm -rf server client

server: server.c message.h message.c socket.h user_info_utils.h user_info_utils.c file_system.h file_system.c file.h status_codes.h constants.h 
	$(CC) $(CFLAGS) -o server server.c message.c user_info_utils.c file_system.c -lpthread -fsanitize=address

client: client.c message.h message.c query_util.h query_util.c file_system.h file_system.h file.h status_codes.h constants.h ui.c 
	$(CC) $(CFLAGS) -o client client.c message.c query_util.c file_system.c ui.c -lpthread -lform -lncurses -fsanitize=address 
