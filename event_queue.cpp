#include "event_queue.h"
#include "heap.h"


static heap <event_t, greater<event_t>> event_queue_;

void push_event(msg_t *msg, time_t t/* = 0*/) {
    event_queue_.push(event_t(t, msg));
}

event_t pop_event() {
    event_queue_.pop();
}

