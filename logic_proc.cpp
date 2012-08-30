#include "logic_proc_cmd.h"

static LogicCmd logic_cmd[] (Msg *msg) = {
    {CMD_LOIGIN, proc_login_cmd},
};

int proc_login_cmd (Msg *msg) {

    return 0;
}
