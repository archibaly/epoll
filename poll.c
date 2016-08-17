#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "poll.h"
#include "hash.h"
#include "debug.h"

#define MAX_EVENTS			64
#define HASH_NUM_BUCKETS	4096

static struct hash_table *epoll_table;

int epoll_new(void)
{
	int epoll_fd;
	epoll_fd = epoll_create(MAX_EVENTS);
	if (epoll_fd < 0) {
		debug("epoll_create(): %s", strerror(errno));
		return -1;
	}
	if (!(epoll_table = hash_init(HASH_NUM_BUCKETS, HASH_KEY_TYPE_INT))) {
		close(epoll_fd);
		return -1;
	}

	return epoll_fd;
}

static poll_event_t *poll_event_new(int fd, uint32_t events)
{
	poll_event_t *event = calloc(1, sizeof(poll_event_t));

	if (event) {
		event->fd = fd;
		event->events = events;
	}

	return event;
}

static int epoll_ctl_mod(int epoll_fd, int fd, uint32_t events)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(struct epoll_event));
	event.data.fd = fd;
	event.events = events;
	return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

static int epoll_ctl_add(int epoll_fd, int fd, uint32_t events)
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
	int n;
	poll_event_t *event;
	struct hash_node *node;

	n = hash_find(epoll_table, &fd, &node, 1);

	if (n > 0) {
		event = node->value;
		event->events |= events;
		return epoll_ctl_mod(epoll_fd, fd, event->events);
	} else {
		if (!(event = poll_event_new(fd, events)))
			return -1;
		if (hash_add(epoll_table, &event->fd, event) < 0) {
			free(event);
			return -1;
		}
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
	int n;
	poll_event_t *event;
	struct hash_node *node;

	n = hash_find(epoll_table, &fd, &node, 1);
	if (n > 0) {
		event = node->value;
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		free(event);
		hash_free_node(node);
	}
}

void poll_event_process(int epoll_fd)
{
	int i;
	poll_event_t *event;
	struct hash_node *node;
	struct epoll_event events[MAX_EVENTS];

	for (;;) {
		int fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		for (i = 0; i < fds; i++) {
			int n = hash_find(epoll_table, &events[i].data.fd, &node, 1);
			if (n > 0) {
				event = node->value;
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
				if ((events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP)) {
					if (event->close_callback)
						event->close_callback(event);
				}
			}
		}
	}
}
