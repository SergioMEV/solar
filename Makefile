CC := clang
CFLAGS := -g

all: main

clean:
	rm -rf ui file_system main

main: main.c ui.o file_system.o Makefile
	$(CC) $(CFLAGS) -o main main.c ui.o file_system.o -fsanitize=address

# Compilation commands:
ui.o: ui.c Makefile
	$(CC) $(CFLAGS) -c -o ui.o ui.c -lncurses

file_system.o: file.h file_system.c Makefile
	$(CC) $(CFLAGS) -c -o file_system.o file_system.c 

# ui: ui.c 
# 	$(CC) $(CFLAGS) -o ui ui.c -lncurses

# file_system: file_system.c
# 	$(CC) $(CFLAGS) -o file_system file_system.c -fsanitize=address