CC = gcc
CFLAGS = -std=c11 -D_DEFAULT_SOURCE -Wall -Wextra -Wpedantic -g

.PHONY: all clean

all: onelanebridge

onelanebridge: onelanebridge.c bridge.h common.h common_threads.h
	$(CC) $(CFLAGS) -pthread onelanebridge.c -o onelanebridge

clean:
	rm -f onelanebridge
