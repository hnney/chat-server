#ifndef __DEF_H__
#define __DEF_H__


#define CONN_INVALID_TIME 50*1000*1000 * 1000;

enum CMD_ {
    CMD_RESERVE = 0,
    CMD_LOGIN = 1, //login
    CMD_GET_FRIEND = 2, //friend group
    CMD_GET_GROUPINFO = 3, //group info
    CMD_EXIT,
    CMD_TEXT, 
    CMD_TRANS_FILE,
    CMD_SHARE_FILE,
    CMD_TRANS_VIDEO,
    CMD_MODIFY_INFO,
    CMD_MODIFY_GROUP_INFO,
    CMD_MODIFY_FRIEND,
    CMD_GETALL_USERS,
    CMD_FIND_USER,
    CMD_ADD_FRIEND,
    CMD_DEL_FRIEND,
    CMD_KA,
};

#endif //__DEF_H__

