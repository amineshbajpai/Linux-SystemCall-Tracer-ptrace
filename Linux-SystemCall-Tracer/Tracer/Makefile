CC = gcc

CFLAGS = -Wall -g

INCLUDES = -Iinclude

SRC = src/tracer.c src/syscalls.c src/process_info.c src/process_table.c src/ui.c

TARGET = tracer

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
