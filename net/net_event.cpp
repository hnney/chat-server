#include "net_event.h"
#include <stdlib.h>
#include <unistd.h>

#ifndef EPOLLRDHUP
#define USE_OWN_RDHUP
#define EPOLLRDHUP 0x2000
#endif

#include <iostream>
using namespace std;

event_t* event_init (_handle_callback callback, int size)
{
    struct event_t* handle;
    int epfd = epoll_create (32000);
    if (epfd < 0) {
        return NULL;
    }
    handle = (struct event_t*)malloc (sizeof (struct event_t));
    if (!handle) {
        return NULL;
    }
    handle->epfd = epfd;
    handle->nevents = size;
    handle->callback = callback;
    handle->events = (struct epoll_event*) malloc (sizeof (epoll_event) * size);  
    if (!handle->events) {
        free (handle);
        return NULL;
    }
    return handle;
}

int event_destroy (event_t *handle)
{
    if (handle) {
        if (handle->epfd > 0) {
            close (handle->epfd);
        }
        if (handle->events) {
            free (handle->events);
        }
        free (handle);
    }
    return 0;
}

int event_add (event_t* handle, int fd, int op)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (op & EV_WRITE) {
        ev.events |= EPOLLOUT;
    }
    if (op & EV_READ) {
        ev.events |= EPOLLIN;
    }
    if (op & EV_ET) {
        ev.events |= EPOLLET;
    }
    if (epoll_ctl (handle->epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        return -1;
    }
    return 0;
}

int event_del (event_t* handle, int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    if (epoll_ctl (handle->epfd, EPOLL_CTL_DEL, fd, &ev) < 0) {
        return -1;
    }
    return 0;
}

int event_modify (event_t* handle, int fd, int op)
{
    struct epoll_event ev;
    ev.data.fd = fd; 
    ev.events = EPOLLERR | EPOLLHUP | EPOLLRDHUP;
    if (op & EV_WRITE) {
        ev.events |= EPOLLOUT;
    }
    if (op & EV_READ) {
        ev.events |= EPOLLIN; 
    }
    if (op & EV_ET) {
        ev.events |= EPOLLET;
    }
    if (epoll_ctl (handle->epfd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        return -1;
    }
    return 0;
}

int event_dispatch (event_t *handle, int timeout) {
    struct epoll_event *events = handle->events;
    //for (;;) {
        int res = epoll_wait (handle->epfd, events, handle->nevents, timeout);
        for (int i = 0; i < res; i++) {
            int op = 0;
            if (events[i].events & (EPOLLERR|EPOLLHUP|EPOLLRDHUP) ) {
                op |= EV_READ | EV_WRITE;
            }
            else {
                if (events[i].events & EPOLLIN) {
                    op |= EV_READ;
                }
                if (events[i].events & EPOLLOUT) {
                    op |= EV_WRITE;
                }
            }
            handle->callback (events[i].data.fd, op);
            //cerr<<"fd:"<<events[i].data.fd<<" event:"<<((op&EV_WRITE)?"write,":"")<<((op&EV_READ)?"read":"")<<endl;;
        }
    //}
    return 0;
}

#ifdef USE_OWN_RDHUP
#undef EPOLLRDHUP
#undef USE_OWN_RDHUP
#endif


