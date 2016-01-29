#ifndef _POLL_H_
#define _POLL_H_

#include "uthash.h"
#include <sys/epoll.h>

typedef struct poll_event poll_event_t;

typedef void (*callback)(const poll_event_t *, struct epoll_event);

struct poll_event {
	int fd;				/* fd to monitor */
	void *data;
	uint32_t events;	/* EPOLLIN, EPOLLET, etc... */
	callback read_callback;
	callback write_callback;
	callback close_callback;
	callback accept_callback;
	UT_hash_handle hh;	/* make poll_event hashable */
};

int epoll_new(void);
int poll_event_add(int epoll_fd, int fd, uint32_t events, poll_event_t **poll_event);
void add_read_callback(poll_event_t *poll_event, callback read_callback);
void add_write_callback(poll_event_t *poll_event, callback write_callback);
void add_close_callback(poll_event_t *poll_event, callback close_callback);
void add_accept_callback(poll_event_t *poll_event, callback accept_callback);
void poll_event_del(int epoll_fd, int fd);
void poll_event_process(int epoll_fd);

#endif	/* _POLL_H_ */
