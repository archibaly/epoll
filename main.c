#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

#include "socket.h"
#include "poll.h"
#include "writen.h"


#define LOCAL_PORT	1991
#define BUFFER_SIZE	1024


static int epoll_fd = -1;


void read_cb(const poll_event_t *poll_event)
{
	char read_buf[BUFFER_SIZE];
	int n = read(poll_event->fd, read_buf, BUFFER_SIZE - 1);
	if (n > 0) {
		read_buf[n] = 0;
		printf("received data: %s\n", read_buf);
		/* write back */
		writen(poll_event->fd, read_buf, n);
	}
}

void close_cb(const poll_event_t *poll_event)
{
#ifdef DEBUG
	printf("in close_cb\n");
#endif
	poll_event_del(epoll_fd, poll_event->fd);
}

void accept_cb(const poll_event_t *poll_event)
{
	struct sockaddr in_addr;
	socklen_t in_len = sizeof(struct sockaddr);
	int connfd = accept(poll_event->fd, &in_addr, &in_len);
	socket_set_non_blocking(connfd);
#ifdef DEBUG
	printf("get the socket %d\n", connfd);
#endif
	poll_event_t *event;
	poll_event_add(epoll_fd, connfd, EPOLLIN | EPOLLRDHUP, &event);
	add_read_callback(event, read_cb);
	add_close_callback(event, close_cb);
}

int main()
{
	int sockfd;

	sockfd = socket_create();
	socket_bind(sockfd, LOCAL_PORT);
	socket_set_non_blocking(sockfd);
	socket_start_listening(sockfd);

	epoll_fd = epoll_new();

	poll_event_t *poll_event;
	poll_event_add(epoll_fd, sockfd, EPOLLIN, &poll_event);
	add_accept_callback(poll_event, accept_cb);

	poll_event_process(epoll_fd);

	return 0;
}
