#ifndef __DEF_H__
#define __DEF_H__


#define CONN_INVALID_TIME 30*1000*1000;

enum CMD_ {
    CMD_RESERVE = 0,
    CMD_LOGIN = 1, //login
    CMD_GET_FRIEND = 2, //friend group
    CMD_GET_GROUPINFO = 3, //group info
    CMD_EXIT = 4,
    CMD_TEXT = 5, 
    CMD_TRANS_FILE = 6,
    CMD_SHARE_FILE = 7,
    CMD_TRANS_VIDEO = 8,
    CMD_MODIFY_INFO = 9,
    CMD_MODIFY_GROUP_INFO = 10,
    CMD_MODIFY_FRIEND = 11,
    CMD_GETALL_USERS = 12,
    CMD_FIND_USER = 13,
    CMD_ADD_FRIEND = 14,
    CMD_DEL_FRIEND = 15,
    CMD_KA = 16,
};

#endif //__DEF_H__

