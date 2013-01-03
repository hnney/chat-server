#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include <vector>
#include "../common/msg.h"

/*
class event_t {
    public:
        time_t time;
        msg_t *msg;

        event_t(time_t t, msg_t *msg) {
            this->time = t;
            this->msg = msg;
        }

        bool operator < (const event_t &obj) {
            return time < obj.time;
        }
        bool operator > (const event_t &obj) {
            return time > obj.time;
        }
};
*/

void push_proc_event(msg_t *msg);
void push_proc_event(vector<msg_t *>& msgs);
msg_t *pop_proc_event();
msg_t *try_pop_proc_event();

void push_send_event(msg_t *msg);
void push_send_event(vector <msg_t *>& msgs);
msg_t *pop_send_event();

#endif //__EVENT_QUEUE_H__

