#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include "msg.h"

class event_t {
    public:
        time_t time;
        msg_t *msg;

        event_t(time_t t, msg_t *msg) {
            this.time = t;
            this.msg = msg;
        }

        bool operator < (const event_t &obj) {
            return time < obj.time;
        }
        bool operator > (const event_t &obj) {
            return time > obj.time;
        }
};

void      push_event(msg_t *msg);
event_t*  pop_event();
int       event_size();
time_t    top_event_time();

#endif //__EVENT_QUEUE_H__

