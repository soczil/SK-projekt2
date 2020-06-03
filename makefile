TARGETS = radio-proxy

CC = g++
CFLAGS = -Wall -Wextra -O2 -g
LFLAGS = -Wall

all: $(TARGETS)

radio-proxy.o: radio-proxy.cpp radio-proxy.h socket.h message.h err.h
	$(CC) $(CFLAGS) -c $<

socket.o: socket.cpp socket.h err.h
	$(CC) $(CFLAGS) -c $<

client.o: client.cpp client.h err.h
	$(CC) $(CFLAGS) -c $<

err.o: err.c err.h
	$(CC) $(CFLAGS) -c $<

radio-proxy: radio-proxy.o socket.o client.o err.o
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o *~
