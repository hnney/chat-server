#ifndef __EVENT_H__
#define __EVENT_H__

#include <string.h>
#include <iostream>

using namespace std;

#include "serialize.h"

class msg_t {
    private:
        int cmd_;   
        int type_;   
        int state_;
        int succ_;

        int buflen_;
        char *buf_;

        string uid_;    
        string tuid_;
        string msg_;

        int user_id_;
	int tuser_id_;

        unsigned int mark_;
        void setbits(int index) { mark_ |= 1<<index; }
        bool hasbits(int index) { return (mark_ & 1<<index) != 0; }

    public:
        msg_t();
        virtual ~msg_t();

        int serialize_size(); 
        int serialize(char *buf);
        int unserialize(char *buf); 
        friend ostream& operator <<(ostream &os, msg_t &e);

        //id = 1
        int cmd() { return cmd_;}
        void set_cmd (int cmd) { 
            cmd_ = cmd;
            setbits(1);
        }
        //id = 2;
        int type() { return type_; }
        void set_type (int type) {
            type_ = type;
            setbits(2);
        }
        //id = 3
        string &uid() { return uid_; }
        void set_uid (string& uid) {
            uid_ = uid;
            setbits(3);
        }
        //id = 4;
        string tuid() { return tuid_;}
        void set_tuid (string tuid) {
            tuid_ = tuid;
            setbits(4);
        }
        //id = 5;
        int buflen () { return buflen_;}
        char *buf () { return buf_;}
        void set_buf(const char *buf, int len) {
            if (buf_) {
                free(buf_);
            }
            buflen_ = len;
            buf_ = (char*)malloc(sizeof(char)*len);
            memcpy (buf_, buf, len);
            setbits(5);
        }
        //id = 6 
        string &msg() { return msg_;}
        void set_msg (string &msg) { 
            msg_ = msg;
            setbits(6);
        }
        //id = 7
        int succ() { return succ_;}
        void set_succ (int succ) {
            succ_ = succ;
            setbits(7);
        }
        //id = 8
        int state() { return state_;}
        void set_state(int state) {
            state_ = state;
            setbits(8);
        }
        //id = 9
        int user_id() { return user_id_;}
        void set_user_id(int id) {
            user_id_ = id;
            setbits(9);
        } 
        //id = 10
        int tuser_id() { return tuser_id_; }
        void set_tuser_id(int tuid) { 
            tuser_id_ = tuid;
            setbits(10);
        }
};

#endif

