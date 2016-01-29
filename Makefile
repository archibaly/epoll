CC = gcc
EXE = epoll

SRCS += main.c
SRCS += socket.c
SRCS += poll.c
SRCS += writen.c

OBJS += main.o
OBJS += socket.o
OBJS += poll.o
OBJS += writen.o

CFLAGS = -Wall -DDEBUG

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

$(OBJS): $(SRCS)

clean:
	-rm -f *.o $(EXE)
