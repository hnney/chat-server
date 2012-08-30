#ifndef __NET_EVENTS_H__
#define __NET_EVENTS_H__

#include <sys/epoll.h>

#define EV_READ     0x01
#define EV_WRITE    0x02
#define EV_PERSIST  0x04
#define EV_ET       0x08


typedef int (*_handle_callback)(int fd, int evop);

struct event_t {
    _handle_callback callback;
    struct epoll_event *events;
    int nevents;
    int epfd;
};

event_t* event_init (_handle_callback callback, int size = 10240); 
int      event_destroy(event_t* handle);
int      event_add (event_t* handle, int fd, int op);
int      event_del (event_t* handle, int fd);
int      event_modify (event_t* handle, int fd, int op);
int      event_dispatch (event_t *handle, int timeout = -1);

#endif //_NET_EVENTS_H__ 

