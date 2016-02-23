CC=gcc
CFLAGS  = -pthread
TARGET = monitor
FILES= monitor.c
build: $(FILES)
	$(CC) $(FILES) -o $(TARGET) $(CFLAGS)

