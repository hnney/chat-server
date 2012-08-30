#ifndef __USER_H__
#define __USER_H__

#include <string>
#include <time.h>

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
        union   data {
            void*   ptr;
            int     uid;
        };
        time_t  invalid_time;
        
};

class conninfo {
    public:
        conn_t *conn;

    public:

        conninfo(conn_t *c) {
            conn = c; 
        }

        bool operator < (const conninfo &obj) {
            return c->invalid_time < obj.c->invalid_time;
        }
        bool operator > (const conn_t &obj) {
            return c->invalid_time > obj.c->invalid_time;
        }

};

class person_t {
    public:
        person_t() {}
        virtual ~person_t() {}
        int     uid;
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

class user_t : public: person_t {
    public:
        user_t(){}
        virtual ~user_t(){}
        vector <friend_t>   friends;
        vector <group_t>    groups;
        void *ptr;
        //other msg
};

#endif //__USER_H__

