#ifndef __LOGIC_PROC_CMD_H__
#define __LOGIC_PROC_CMD_H__

#include "../common/msg.h"
#include "../common/common.h"
#include "../json/json_util.h"

typedef int (*cmd_callback)(msg_t *msg, conn_t *conn);

typedef struct _logicCmd {
    int cmd_;
    cmd_callback callback_;
}LogicCmd;

int proc_cmd(msg_t *msg, conn_t *conn);
int proc_login_cmd(msg_t *msg, conn_t *conn);
int proc_exit_cmd(msg_t *msg, conn_t *conn);
int proc_keepalive_cmd(msg_t *msg, conn_t *conn); 
int proc_text_cmd(msg_t *msg, conn_t *conn);

void clean_conn(conn_t *conn);
void clean_user(user_t *user);
void send_user_exit(user_t *user);
void send_keepalive(conn_t *conn);

#endif //__LOGIC_PROC_CMD_H__

