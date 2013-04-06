#ifndef __USER_H__
#define __USER_H__

#include <vector>
#include <string>
#include <time.h>
#include <iostream>

#include <log4cxx/logger.h>

#include "config.h"

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
        void*   ptr;
        /*
        union   _data {
            void*   ptr;
            int     uid;
        }data;
        */
        time_t  invalid_time;
        int     ip;
        
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

#define STATE_WAIT_LOGIN 0 
#define STATE_LOGINED 1
#define STATE_EXIT 2
#define STATE_DISCONNECT 3
#define STATE_AUTH_FAILED 4

class user_t {
    public:
        string uid;
        int id;
        int invited;
        
        vector <int> friend_ids;
        vector <int> group_ids;
        vector <int> talk_ids; 

        user_t() {
            conn = NULL;
            state = STATE_EXIT;
        }
        conn_t *conn;
        int state;

        void del_group(int group_id) {
            for (vector<int>::iterator iter = group_ids.begin(); iter != group_ids.end(); iter++) {
                if (*iter == group_id) {
                    group_ids.erase(iter);
                    break;
                } 
            }
        }
};

class group_t {
    public:
        int id;
        string name; //uid
        vector <int> member_ids;
};

class talk_t {
    public:
        int id;
        string name; //uid
        vector <int> talk_ids;
};

#endif //__USER_H__


