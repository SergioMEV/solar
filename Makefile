CC := clang
CFLAGS := -g

all: ui

clean:
	rm -rf ui

ui: ui.c 
	$(CC) $(CFLAGS) -o ui ui.c -lncurses