TARGETS = radio-proxy

CC = g++
CFLAGS = -Wall -Wextra -O2 -g
LFLAGS = -Wall

all: $(TARGETS)

radio-proxy.o: radio-proxy.h err.h

socket.o: socket.h err.h

radio-proxy: radio-proxy.o socket.o err.o

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o *~
