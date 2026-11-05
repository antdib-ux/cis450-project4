# Keep compilation consistent on Ubuntu and rebuild when any header changes.
CC = gcc
CFLAGS = -std=c11 -D_DEFAULT_SOURCE -Wall -Wextra -Wpedantic -g

.PHONY: all check clean

all: onelanebridge

onelanebridge: onelanebridge.c bridge.h common.h common_threads.h
	$(CC) $(CFLAGS) -pthread onelanebridge.c -o onelanebridge

# Check the framework without running the unfinished crossing policies.
check: onelanebridge
	./onelanebridge --check

# Remove only the generated executable.
clean:
	rm -f onelanebridge
