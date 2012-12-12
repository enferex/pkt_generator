APP=pkt_generator
CC=gcc
CFLAGS=-g3 -Wall -pedantic -std=c99

$(APP): main.c
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f $(APP)
