CC = cc
EXE = epoll

CFLAGS = -Wall #-DDEBUG
LDFLAGS = -lm

$(EXE): main.o socket.o poll.o writen.o config.o hash.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f *.o tags $(EXE)
