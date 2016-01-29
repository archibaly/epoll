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

int epoll_event_mod(int epoll_fd, int fd)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(struct epoll_event));
	event.fd = fd;
	event.events = event->events;
	return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

int epoll_event_add(int epoll_fd, int fd)
{
	struct epoll_event event;
	memset(&event, 0, sizeof(struct epoll_event));
	event.fd = fd;
	event.events = event->events;
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
		return epoll_event_mod(epoll_fd, fd);
	} else {
		event = poll_event_new(fd, events);
		HASH_ADD_INT(hash_head, fd, event);
		*poll_event = event;
		return epoll_event_add(epoll_fd, fd);
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
		HASH_DEL(hash_head, event);
		close(fd);
		free(event);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	}
}

int poll_event_process(const poll_event_t *poll_event)
{
}
