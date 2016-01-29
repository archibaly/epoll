CC = gcc
EXE = epoll

SRCS += main.c
SRCS += socket.c

OBJS += main.o
OBJS += socket.o

CFLAGS = -Wall -DDEBUG

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJS): $(SRCS)

clean:
	-rm -f tags *.o $(EXE)
