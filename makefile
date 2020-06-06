TARGETS = radio-proxy radio-client

CC = g++
CFLAGS = -Wall -Wextra -O2
LFLAGS = -Wall

all: $(TARGETS)

radio-client.o: radio-client.cpp radio-client.h message.h socket.h server.h telnet-screen.h err.h
	$(CC) $(CFLAGS) -c $<

radio-proxy.o: radio-proxy.cpp radio-proxy.h socket.h message.h client.h err.h
	$(CC) $(CFLAGS) -c $<

server.o: server.cpp server.h
	$(CC) $(CFLAGS) -c $<

socket.o: socket.cpp socket.h err.h
	$(CC) $(CFLAGS) -c $<

client.o: client.cpp client.h err.h
	$(CC) $(CFLAGS) -c $<

telnet-screen.o: telnet-screen.cpp telnet-screen.h server.h err.h
	$(CC) $(CFLAGS) -c $<

err.o: err.c err.h
	$(CC) $(CFLAGS) -c $<

radio-client: radio-client.o socket.o server.o telnet-screen.o err.o
	$(CC) $(CFLAGS) -pthread -o $@ $^

radio-proxy: radio-proxy.o socket.o client.o err.o
	$(CC) $(CFLAGS) -pthread -o $@ $^

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o *~
