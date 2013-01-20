#ifndef __LOGIC_PROC_CMD_H__
#define __LOGIC_PROC_CMD_H__

#include "../common/msg.h"
#include "../common/common.h"
#include "../json/json_util.h"
#include "../common/dbstruct.h"


typedef int (*cmd_callback)(msg_t *msg, void *arg);

typedef struct _logicCmd {
    int cmd_;
    cmd_callback callback_;
}LogicCmd;

int proc_cmd(msg_t *msg, void *arg);
int proc_login_cmd(msg_t *msg, void *arg);
int proc_exit_cmd(msg_t *msg, void *arg);
int proc_text_cmd(msg_t *msg, void *arg);
int proc_keepalive_cmd(msg_t *msg, void *arg); 
int proc_find_info(msg_t *msg, void *arg);
int proc_add_friend(msg_t *msg, void *arg);
int proc_del_friend(msg_t *msg, void *arg);

int proc_load_messages_cmd(msg_t *msg, void *arg);

void clean_conn(conn_t *conn);
void send_keepalive();


#endif //__LOGIC_PROC_CMD_H__

