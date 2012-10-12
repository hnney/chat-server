#include "data_proc.h"
#include "../net/net_func.h"
#include "../common/utils.h"
#include "../event/event_queue.h"

static LogicCmd logic_cmd[] = {
    {CMD_LOGIN, proc_login_cmd},
    {CMD_EXIT, proc_exit_cmd},
    {CMD_KA, proc_keepalive_cmd},
};

int proc_cmd(msg_t* msg, void *arg) {
    int ret = -1;
    if (msg->cmd() >= 0 && (unsigned int)msg->cmd() < sizeof(logic_cmd)/sizeof(logic_cmd[0])) {
        ret = (*(logic_cmd[msg->cmd()].callback_))(msg, arg);
    }
    else {
        cerr<<"proc_cmd: err msg cmd:"<<msg->cmd()<<endl;
    }
    return ret;
}

void send_keepalive(conn_t *conn, msg_t *msg) {
    //TODO
}

int proc_login_cmd (msg_t *msg, void *arg) {
    return 0;
}

int proc_exit_cmd(msg_t* msg, void *arg) {
    return 0; 
}

int proc_keepalive_cmd(msg_t *msg, void *arg) {
    return 0;
}



