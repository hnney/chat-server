#ifndef __LOGIC_PROC_CMD_H__
#define __LOGIC_PROC_CMD_H__

#include "../message/msg.h"

typedef int (*cmd_callback)(Msg *msg);

typedef struct _logicCmd {
    int cmd_;
    cmd_callback callback_;
}LogicCmd;

enum CMD_ {
    CMD_LOGIN = 0, //login
};


#endif //__LOGIC_PROC_CMD_H__

