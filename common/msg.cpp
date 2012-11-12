#include "msg.h"
#include <iostream>

#include <arpa/inet.h>
using namespace std;

msg_t::msg_t() {
    mark_ = 0;
    cmd_ = 0;
    buflen_ = 0;
    buf_ = 0;
    succ_ = 0;
    buf_ = NULL;
}
msg_t::~msg_t() {
    if (buf_) {
        free(buf_);
    }
}
int msg_t::serialize_size() {
    int ret = sizeof(mark_);
    if (hasbits(1)) {
        ret += sizeof(cmd_);
    }
    if (hasbits(2)) {
        ret += sizeof(type_);
    }
    if (hasbits(3)) {
        ret += sizeof(int);
        ret += uid_.size();
    }
    if (hasbits(4)) {
        ret += sizeof(int);
        ret += tuid_.size();
    }
    if (hasbits(5)) {
        ret += sizeof(buflen_);
        ret += sizeof(char)*buflen_;
    }
    if (hasbits(6)) {
        ret += sizeof(int);
        ret += msg_.size();
    }
    if (hasbits(7)) {
        ret += sizeof(succ_);
    }
    if (hasbits(8)) {
        ret += sizeof(state_);
    }
    return ret;
}
int msg_t::serialize(char *buf) {
    char *data = buf;
    saveuint(data, htonl(mark_));
    if (hasbits(1)) {
        saveint(data,htonl(cmd_));
    }
    if (hasbits(2)) {
        saveint(data,htonl(type_));
    }
    if (hasbits(3)) {
        savestring(data,uid_);
    }
    if (hasbits(4)) {
        savestring(data,tuid_);
    }
    if (hasbits(5)) {
        saveint(data,htonl(buflen_));
        if (buflen_ > 0) {
            savebytes(data,buf_,buflen_);
        }
    }
    if (hasbits(6)) {
        savestring(data, msg_);
    }
    if (hasbits(7)) {
        saveint(data, htonl(succ_));
    }
    if (hasbits(8)) {
        saveint(data, htonl(state_));
    }
    return data-buf;
}
int msg_t::unserialize(char *buf) {
    char *data = buf;
    mark_ = ntohl(loaduint(data));
    if (hasbits(1)) {
        cmd_ = ntohl(loadint(data));
    }
    if (hasbits(2)) {
        type_ = ntohl(loadint(data));
    }
    if (hasbits(3)) {
        uid_ = loadstring(data);
    }
    if (hasbits(4)) {
        tuid_ = loadstring(data);
    }
    if (hasbits(5)) {
        buflen_ = ntohl(loadint(data));
        buf_ = NULL;
        if (buflen_ > 0) {
            buf_ = (char*)malloc(sizeof(char)*buflen_);
            loadbytes(data, buf_, buflen_);
        }
    }
    if (hasbits(6)) {
        msg_ = loadstring(data); 
    }
    if (hasbits(7)) {
        succ_ = ntohl(loadint(data));
    }
    if (hasbits(8)) {
        state_ = ntohl(loadint(data));
    }
    return data-buf;
}

ostream& operator <<(ostream &os, msg_t &e) {
    if(e.hasbits(1)) {
        os<<"cmd:"<<(e.cmd())<<endl;
    }
    if (e.hasbits(2)) {
        os<<"type:"<<e.type()<<endl;
    }
    if (e.hasbits(3)) {
        os<<"uid:"<<e.uid().c_str()<<endl;
    }
    if (e.hasbits(4)) {
        os<<"tuid:"<<e.tuid().c_str()<<endl;
    }
    if (e.hasbits(5)) {
        os<<"buflen:"<<e.buflen()<<endl;//<<",buf:"<<e.buf()<<endl;
    }
    if (e.hasbits(6)) {
        os<<"msg: "<<e.msg()<<endl;
    }
    if (e.hasbits(7)) {
        os<<"succ: "<<e.succ()<<endl;
    }
    if (e.hasbits(8)) {
        os<<"sate: "<<e.state()<<endl;
    }
    return os;
}


#ifdef __TEST__
int main (int argc, char **argv) {

    msg_t e;
    string uid = "10";
    e.set_cmd(1);
    e.set_type(2);
    e.set_uid(uid);
    e.set_buf("good", 4);

    int len = e.serialize_size();
    cout<<"buf len:"<<len<<endl;
    cout<<"before:\n"<<e<<endl;
    char *buf = (char*)malloc(len);
    e.serialize(buf);

    msg_t e1;
    char *buf1 = (char*)malloc(len);
    memcpy(buf1, buf, len);
    e1.unserialize(buf1);
    cout<<"after:\n"<<e1<<endl;

    return 0;
}

#endif // __TEST__

