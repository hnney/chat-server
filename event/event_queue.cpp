#include "event_queue.h"
#include "../common/heap.h"

#include <deque>
#include <vector>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

//处理事件队列
static deque<msg_t*> event_proc_;
//发送事件队列
static deque<msg_t*> event_send_;

static pthread_mutex_t proc_mutex_ = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t send_mutex_ = PTHREAD_MUTEX_INITIALIZER;

static sem_t sem;

int event_init() {
    pthread_mutex_init(&proc_mutex_, NULL);
    pthread_mutex_init(&send_mutex_, NULL);
    sem_init(&sem, 0, 0);
    return 0;
}

void push_proc_event(msg_t *msg) {
    pthread_mutex_lock(&proc_mutex_);
    event_proc_.push_back(msg);
    sem_post(&sem);
    pthread_mutex_unlock(&proc_mutex_);
}
void push_proc_event(vector <msg_t*> &msgs) {
    pthread_mutex_lock(&proc_mutex_);
    for (size_t i = 0; i < msgs.size(); i++) {
        event_proc_.push_back(msgs[i]);
        sem_post(&sem);
    }
    pthread_mutex_unlock(&proc_mutex_);
}
msg_t *pop_proc_event() {
    sem_wait(&sem);
    msg_t *msg = NULL;
    pthread_mutex_lock(&proc_mutex_);
    if (event_proc_.size() > 0) {
        msg = event_proc_.front();
        event_proc_.pop_front();
    }
    pthread_mutex_unlock(&proc_mutex_);
    return msg;
}

void push_send_event(msg_t *msg) {
    pthread_mutex_lock(&send_mutex_);
    event_send_.push_back(msg);
    pthread_mutex_unlock(&send_mutex_);
}

msg_t *pop_send_event() {
    msg_t *msg = NULL;
    pthread_mutex_lock(&send_mutex_);
    if (event_send_.size() > 0) {
        msg = event_send_.front();
        event_send_.pop_front();
    }
    pthread_mutex_unlock(&send_mutex_);
    return msg;
}

