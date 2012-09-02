#include "logic_proc.h"

static LogicCmd logic_cmd[] (Msg *msg, conn_t *conn) = {
    {CMD_LOIGIN, proc_login_cmd},
};

int proc_cmd(Msg* msg, conn_t *conn) {
    if (msg->cmd >= 0 && msg->cmd < sizeof(logic_cmd)) {
        (*(logic_cmd[msg->cmd].callback_))(msg)
    }
    else {
        cerr<<"proc_cmd: err msg cmd:"<<msg-cmd<<endl;
    }
}

int proc_login_cmd (Msg *msg, conn_t *conn) {

    return 0;
}

