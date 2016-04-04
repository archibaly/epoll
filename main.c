#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#include "debug.h"
#include "socket.h"
#include "poll.h"
#include "writen.h"
#include "config.h"

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
	INFO("in close_cb\n");
	poll_event_del(epoll_fd, poll_event->fd);
}


void accept_cb(const poll_event_t *poll_event)
{
	struct sockaddr in_addr;
	socklen_t in_len = sizeof(struct sockaddr);
	int connfd = accept(poll_event->fd, &in_addr, &in_len);
	socket_set_non_blocking(connfd);
	INFO("get the socket %d\n", connfd);
	poll_event_t *event;
	poll_event_add(epoll_fd, connfd, EPOLLIN | EPOLLRDHUP, &event);
	add_read_callback(event, read_cb);
	add_close_callback(event, close_cb);
}


int main()
{
	int sockfd;
	unsigned short port;

	config_load("epoll.conf");
	port = atoi(config_get_value("server_port"));

	sockfd = socket_create();
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
