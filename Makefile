CC := clang
CFLAGS := -g

all: ui

clean:
	rm -rf ui file_system

ui: ui.c 
	$(CC) $(CFLAGS) -o ui ui.c -lncurses

file_system: file_system.c
	$(CC) $(CFLAGS) -o file_system file_system.c -fsanitize=address