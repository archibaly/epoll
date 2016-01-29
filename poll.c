#include <unistd.h>
#include <stdio.h>

#include "poll.h"

#define MAX_EVENTS	64

static poll_event_t *hash_head = NULL;

int epoll_new(void)
{
	int epoll_fd;
	epoll_fd = epoll_create(MAX_EVENTS);
	if (epoll_fd == -1) {
	#ifdef DEBUG
		perror("epoll_create");
	#endif
		abort();
	}
	return epoll_fd;
}

poll_event_t *poll_event_new(int fd, uint32_t events)
{
	poll_event_t *event = calloc(1, sizeof(poll_event_t));

	if (event) {
		event->fd = fd;
		event->events = events;
	}

	return event;
}

int epoll_ctl_mod(int epoll_fd, int fd, uint32_t events)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(struct epoll_event));
	event.data.fd = fd;
	event.events = events;
	return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

int epoll_ctl_add(int epoll_fd, int fd, uint32_t events)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(struct epoll_event));
	event.data.fd = fd;
	event.events = events;
	return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

/*
 * out: poll_event
 */
int poll_event_add(int epoll_fd, int fd, uint32_t events, poll_event_t **poll_event)
{
	poll_event_t *event;
	HASH_FIND_INT(hash_head, &fd, event);
	if (event) {
		event->events |= events;
		return epoll_ctl_mod(epoll_fd, fd, event->events);
	} else {
		event = poll_event_new(fd, events);
		HASH_ADD_INT(hash_head, fd, event);
		*poll_event = event;
		return epoll_ctl_add(epoll_fd, fd, event->events);
	}
}

void add_read_callback(poll_event_t *poll_event, callback read_callback)
{
	if (poll_event)
		poll_event->read_callback = read_callback;
}

void add_write_callback(poll_event_t *poll_event, callback write_callback)
{
	if (poll_event)
		poll_event->write_callback = write_callback;
}

void add_close_callback(poll_event_t *poll_event, callback close_callback)
{
	if (poll_event)
		poll_event->close_callback = close_callback;
}

void add_accept_callback(poll_event_t *poll_event, callback accept_callback)
{
	if (poll_event)
		poll_event->accept_callback = accept_callback;
}

void poll_event_del(int epoll_fd, int fd)
{
	poll_event_t *event;
	HASH_FIND_INT(hash_head, &fd, event);
	if (event) {
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		HASH_DEL(hash_head, event);
		free(event);
	}
}

void poll_event_process(int epoll_fd)
{
	struct epoll_event events[MAX_EVENTS];
	for (;;) {
		int fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		int i;
		for (i = 0; i < fds; i++) {
			poll_event_t *event;
			HASH_FIND_INT(hash_head, &events[i].data.fd, event);
			if (event) {
				if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI)) {
					if (event->accept_callback)
						event->accept_callback(event);
					if (event->read_callback)
						event->read_callback(event);
				}
				if (events[i].events & EPOLLOUT) {
					if (event->write_callback)
						event->write_callback(event);
				}
				if ((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
					if (event->close_callback)
						event->close_callback(event);
				}
			}
		}
	}
}
