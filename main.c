#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "debug.h"
#include "socket.h"
#include "poll.h"
#include "writen.h"
#include "config.h"

/* #define BUFFER_SIZE	4096 */
#define BUFFER_SIZE	8

static int epoll_fd = -1;

void dump_hex(const unsigned char* data, int len, const char* txt)
{
	int i;
	if (txt != NULL)
		printf("%s: ", txt);
	for (i=0; i < len; i++) {
		if ((i % 2) == 0)
			printf(" ");
		if ((i % 16) == 0)
			printf("\n");
		printf("%02x", data[i]);
	}
	printf("\n\n");
}


void read_cb(const poll_event_t *poll_event)
{
	int n;
	unsigned char read_buf[BUFFER_SIZE];

	for (;;) {
		n = read(poll_event->fd, read_buf, BUFFER_SIZE - 1);
		if (n > 0) {
			printf("received %d bytes:", n);
			dump_hex(read_buf, n, NULL);
		} else if (n < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				debug("read done");
				break;
			}
			debug("read(): %s", strerror(errno));
			break;
		}
	}
}


void close_cb(const poll_event_t *poll_event)
{
	debug("in close_cb");
	poll_event_del(epoll_fd, poll_event->fd);
}


void accept_cb(const poll_event_t *poll_event)
{
	int connfd = socket_accept(poll_event->fd, NULL, 0);
	if (connfd < 0) {
		debug("socket_accept error: %s", strerror(errno));
		return;
	}

	debug("get socket %d", connfd);

	socket_set_non_blocking(connfd);

	poll_event_t *event;

	poll_event_add(epoll_fd, connfd, EPOLLIN | EPOLLET | EPOLLRDHUP, &event);
	add_read_callback(event, read_cb);
	add_close_callback(event, close_cb);
}


int main()
{
	int port;
	int sockfd;

	config_load("epoll.conf");
	port = atoi(config_get_value("server_port"));

	sockfd = socket_create(TCP);
	socket_bind(sockfd, port);
	socket_set_non_blocking(sockfd);
	socket_start_listening(sockfd);

	epoll_fd = epoll_new();

	poll_event_t *poll_event;
	poll_event_add(epoll_fd, sockfd, EPOLLIN, &poll_event);
	add_accept_callback(poll_event, accept_cb);

	poll_event_process(epoll_fd);

	return 0;
}
