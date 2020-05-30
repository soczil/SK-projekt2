TARGETS = radio-proxy

CC = g++
CFLAGS = -Wall -Wextra -O2
LFLAGS = -Wall

all: $(TARGETS)

radio-proxy.o: radio-proxy.h err.h

radio-proxy: radio-proxy.o err.o

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o *~
