CC := clang
CFLAGS := -g
SOURCE := main.c ui.c file_system.c
OBJ := $(SOURCE:.c=.o)

all: main

clean:
	rm -rf main $(OBJ) *.d

main: $(OBJ) Makefile
	$(CC) $(CFLAGS) -o main $(OBJ) -fsanitize=address -lncurses

%.o : %.c
	$(CC) -MD -c -o $@ $<

-include *.d
# # Compilation commands:
# ui.o: ui.c ui.h Makefile
# 	$(CC) $(CFLAGS) -c -o ui.o ui.c

# file_system.o: file.h file_system.c file_system.h Makefile
# 	$(CC) $(CFLAGS) -c -o file_system.o file_system.c 

# ui: ui.c 
# 	$(CC) $(CFLAGS) -o ui ui.c -lncurses

# file_system: file_system.c
# 	$(CC) $(CFLAGS) -o file_system file_system.c -fsanitize=address