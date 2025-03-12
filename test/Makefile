# Makefile for timed test clients

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -std=c99 -D_POSIX_C_SOURCE=200809L

all: test_client test_client_udp

test_client: test_client.c
	$(CC) $(CFLAGS) test_client.c -o test_client

test_client_udp: test_client_udp.c
	$(CC) $(CFLAGS) test_client_udp.c -o test_client_udp

clean:
	rm -f test_client test_client_udp