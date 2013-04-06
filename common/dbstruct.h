#ifndef __DB_STRUCT_H__
#define __DB_STRUCT_H__

#include <string>
#include <vector>

using namespace std;

class DBUser {
public:

    DBUser() {
        user_id = 0;
        state = 0;
        invited = 0;
        subtype = 0;
    }
    int user_id;
    string user_name;
    string user_pwd;
    string user_pwd_key;

    string type; //用户类型
    string truename;
    string sex;
    string height;
    string weight;
    string job;
    string place; 
    string birthday;
    string headurl;
    string experience;
    string user_desc;

    int subtype; //当是群成员是，属于什么类型成员, 0:普通成员，1：普通管理员，2：创建者

    int state;
    int invited;
};

class DBFriend {
public:
    string type;
    DBUser dbuser;
};

//typedef class DBGroup DBTalks;

class DBGroup {
public:
    DBGroup() {
       group_id = 0;
    }
    int group_id;
    int admin_id;
    string name;
    string notice;
    string headurl;
    vector <DBUser> members;
};

class DBTalks {
public:
    DBTalks() {
       talk_id = 0;
    }
    int talk_id;
    string name;
    string notice;
    string headurl;
    vector <DBUser> members;
};

class DBInterface {
public:
    DBUser dbuser;
    vector <DBFriend> dbfriends;
    vector <DBGroup> dbgroups;
    vector <DBTalks> dbtalks;
};

#endif //end

