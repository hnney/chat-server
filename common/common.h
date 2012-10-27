#ifndef __USER_H__
#define __USER_H__

#include <vector>
#include <string>
#include <time.h>
#include <iostream>

#include <log4cxx/logger.h>

#include "config.h"


extern AppConfig config_;
extern log4cxx::LoggerPtr logger_;



#define CONN_DB_SERVER 1
#define CONN_CLIENT    2

using namespace std;
class conn_t {
    private:
        conn_t() {
        }
    public:
        int     fd;
        char*   readbuf;
        char*   writebuf;
        int     readbuf_size;
        int     writebuf_size;
        int     read_pos;
        int     write_pos;
        int     invalid;
        int     mark;
        union   _data {
            void*   ptr;
            int     uid;
        }data;
        time_t  invalid_time;
        
};

class conninfo {
    public:
        conn_t *conn;

        conninfo(conn_t *c) {
            conn = c; 
        }
        bool operator < (const conninfo &obj) const {
            return conn->invalid_time < obj.conn->invalid_time;
        }
        bool operator > (const conninfo &obj) const {
            return conn->invalid_time > obj.conn->invalid_time;
        }
};

class person_t {
    public:
        person_t() {}
        virtual ~person_t() {}
        string  uid;
        string  nickname;
};

class friend_t : public person_t {
    public:
        friend_t(){}
        virtual ~friend_t(){}
        string  type;
        string  remark;
};

class group_t : public person_t {
    public:
        group_t(){}
        virtual ~group_t(){}
        vector <person_t>   members;
};

#define STATE_WAIT_LOGIN 0 
#define STATE_LOGINED 1
#define STATE_EXIT 2
#define STATE_DISCONNECT 3
#define STATE_AUTH_FAILED 4

class user_t : public person_t {
    public:
        user_t(){
            conn = NULL;
        }
        virtual ~user_t(){}
        vector <friend_t>   friends;
        vector <group_t>    groups;
        conn_t* conn;
        int     state;
        //other msg
};

#endif //__USER_H__


