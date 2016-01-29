#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>

#include "socket.h"

#define MAXEVENTS 64

int main(int argc, char *argv[])
{
	int sockfd, ret;
	int epfd;
	struct epoll_event event;
	struct epoll_event *events;

	if (argc != 2) {
		fprintf(stderr, "usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	sockfd = socket_create();
	socket_bind(sockfd, atoi(argv[1]));
	socket_set_non_blocking(sockfd);
	socket_start_listening(sockfd);

	epfd = epoll_create(1);
	if (epfd == -1) {
	#ifdef DEBUG
		perror("epoll_create");
	#endif
		abort();
	}

	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
	if (ret == -1) {
	#ifdef DEBUG
		perror("epoll_ctl");
	#endif
		abort();
	}

	/* Buffer where events are returned */
	events = calloc(MAXEVENTS, sizeof(event));

	/* The event loop */
	for (;;) {
		int n, i;

		n = epoll_wait(epfd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) ||
				(!(events[i].events & EPOLLIN))) {
				/* An error has occured on this fd, or the socket is not
				   ready for reading (why were we notified then?) */
			#ifdef DEBUG
				fprintf(stderr, "epoll error\n");
			#endif
				close(events[i].data.fd);
				continue;
			} else if (sockfd == events[i].data.fd) {
				/* We have a notification on the listening socket, which
				   means one or more incoming connections. */
				for (;;) {
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

					in_len = sizeof(in_addr);
					infd = accept(sockfd, &in_addr, &in_len);
					if (infd == -1) {
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
							/* We have processed all incoming
							   connections. */
							break;
						} else {
						#ifdef DEBUG
							perror("accept");
						#endif
							break;
						}
					}

					ret = getnameinfo(&in_addr, in_len,
									hbuf, sizeof(hbuf),
									sbuf, sizeof(sbuf),
									NI_NUMERICHOST | NI_NUMERICSERV);
					if (ret == 0) {
						printf("Accepted connection on descriptor %d "
							   "(host = %s, port = %s)\n", infd, hbuf, sbuf);
					}

					/* Make the incoming socket non-blocking and add it to the
					   list of fds to monitor. */
					socket_set_non_blocking(infd);

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					ret = epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event);
					if (ret == -1) {
					#ifdef DEBUG
						perror("epoll_ctl");
					#endif
						abort();
					}
				}
				continue;
			} else {
				/* We have data on the fd waiting to be read. Read and
				   display it. We must read whatever data is available
				   completely, as we are running in edge-triggered mode
				   and won't get a notification again for the same
				   data. */
				int done = 0;

				for (;;) {
					ssize_t count;
					char buf[512];

					count = read(events[i].data.fd, buf, sizeof(buf));
					if (count == -1) {
						/* If errno == EAGAIN, that means we have read all
						   data. So go back to the main loop. */
						if (errno != EAGAIN) {
						#ifdef DEBUG
							perror("read");
						#endif
							done = 1;
						}
						break;
					} else if (count == 0) {
						/* End of file. The remote has closed the
						   connection. */
						done = 1;
						break;
					}

					/* Write the buffer to standard output */
					ret = write(STDOUT_FILENO, buf, count);
					if (ret == -1) {
					#ifdef DEBUG
						perror("write");
					#endif
						abort();
					}
				}

				if (done) {
					printf("Closed connection on descriptor %d\n",
						   events[i].data.fd);

					/* Closing the descriptor will make epoll remove it
					   from the set of descriptors which are monitored. */
					close(events[i].data.fd);
				}
			}
		}
	}

	free(events);

	close(sockfd);

	return EXIT_SUCCESS;
}
