CC = gcc
EXE = epoll

CFLAGS = -Wall -DDEBUG

$(EXE): main.o socket.o poll.o writen.o config.o
	$(CC) -o $@ $^

main.o: main.c socket.h poll.h writen.h
socket.o: socket.c socket.h
poll.o: poll.c poll.h
writen.o: writen.c writen.h
config.o: config.c config.h debug.h

clean:
	-rm -f *.o $(EXE)
