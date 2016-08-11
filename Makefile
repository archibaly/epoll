CC = gcc
EXE = epoll

CFLAGS = -Wall -DDEBUG

$(EXE): main.o socket.o poll.o writen.o config.o hash.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o tags $(EXE)
