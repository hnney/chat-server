#include "data_proc.h"
#include "../net/net_func.h"
#include "../common/utils.h"
#include "../event/event_queue.h"
#include "../common/md5.h"
#include "../common/dbmanager.h"
#include "../json/json_util.h"
#include "../common/msginterface.h"
#include "../common/def.h"
#include <assert.h>
#include <time.h>

extern AppConfig config_;
extern log4cxx::LoggerPtr logger_;
static LogicCmd logic_cmd[] = {
    {CMD_RESERVE, NULL}, //0
    {CMD_LOGIN, proc_login_cmd}, //1
    {CMD_GET_FRIEND, NULL}, //2
    {CMD_GET_GROUPINFO, NULL}, //3
    {CMD_EXIT, proc_exit_cmd}, //4
    {CMD_TEXT, proc_text_cmd}, //5
    {CMD_TRANS_FILE, NULL}, //6
    {CMD_SHARE_FILE, NULL}, //7
    {CMD_TRANS_VIDEO, NULL}, //8
    {CMD_MODIFY_INFO, NULL}, //9
    {CMD_MODIFY_GROUP_INFO, NULL}, //10
    {CMD_MODIFY_FRIEND, NULL}, //11
    {CMD_GETALL_USERS, NULL}, //12
    {CMD_FIND_USER, proc_find_info}, //13
    {CMD_ADD_FRIEND, proc_add_friend}, //14
    {CMD_DEL_FRIEND, proc_del_friend}, //15
    {CMD_GROUP_INFO, proc_group_info}, //16
    {CMD_TALK_INFO, NULL}, //17
    {CMD_KA, proc_keepalive_cmd}, //18
    {CMD_LOAD_MESSAGES, proc_load_messages_cmd}, //19
    {CMD_REPORT, proc_report_cmd}, //20
    {CMD_SETINFO, proc_setinfo_cmd}, //21
};

int proc_cmd(msg_t* msg, void *arg) {
    int ret = -1;
    if (msg->cmd() > 0 && (unsigned int)msg->cmd() < sizeof(logic_cmd)/sizeof(logic_cmd[0])) {
        ret = (*(logic_cmd[msg->cmd()].callback_))(msg, arg);
    }
    else {
        cerr<<"proc_cmd: err msg cmd:"<<msg->cmd()<<endl;
    }
    return ret;
}

void send_keepalive() {
    //TODO
    msg_t *msg = new msg_t();
    msg->set_cmd(CMD_KA);
    push_send_event(msg);
}

void record_to_db(msg_t *msg, DBManager *dbm) {
    int size = msg->serialize_size();
    char *buf = (char *) malloc (size * sizeof(char));
    assert(buf != NULL);
    if (msg->serialize(buf) == size) {
        string message(buf, size);
        dbm->setUserMessages(msg->user_id(), message);
    }
    else {
        cerr<<msg->cmd()<<" serialize msg failed"<<endl;
    }
    free(buf);
}

void record_to_db(int user_id, msg_t *msg, DBManager *dbm) {
    int size = msg->serialize_size();
    char *buf = (char *) malloc (size * sizeof(char));
    assert(buf != NULL);
    if (msg->serialize(buf) == size) {
        string message(buf, size);
        dbm->setUserMessages(user_id, message);
    }
    else {
        cerr<<msg->cmd()<<" serialize msg failed"<<endl;
    }
    free(buf);
}

int proc_login_cmd (msg_t *msg, void *arg) {
    if (arg == NULL || msg == NULL) return -1;

    DBManager *dbm = (DBManager *)arg;
    int succ = 1;  
    if (msg->state() == 2) {
        Value json = parseJsonStr(msg->msg());
        DBInterface dbinterface;

        //check auth 
	if (!dbm->getUser(msg->uid().c_str(), dbinterface.dbuser)) {
            succ = 2;
        }
        else {
            //id,name,pwd,pwdkey
            //check pwd
            string pwdsrc = json["pwd"].asString() + dbinterface.dbuser.user_pwd_key;
            string pwdmd5 = md5(pwdsrc);
            if (pwdmd5 == dbinterface.dbuser.user_pwd) {
                succ = 0;
            }
	    else {
		succ = 2;
            }
        }   
         
        if (succ == 0) {
            dbm->setUserState(dbinterface.dbuser.user_id, STATE_LOGINED);
            msg->set_user_id(dbinterface.dbuser.user_id);
            //get_user_info
            if (!dbm->getUserInfo(dbinterface.dbuser.user_id, dbinterface.dbuser)) {
                //TODO LOG
            }

            //get friends
            if (!dbm->getFriends(dbinterface.dbuser.user_id, dbinterface.dbfriends)) {
                //LOG
            }
            //get friend info
            for (size_t i = 0; i < dbinterface.dbfriends.size(); i++) {
                //friend state
                if (!dbm->getUserState(dbinterface.dbfriends[i].dbuser.user_id, dbinterface.dbfriends[i].dbuser)) {
                    //log
                }
                //friend info
                if (!dbm->getUserInfo(dbinterface.dbfriends[i].dbuser.user_id, dbinterface.dbfriends[i].dbuser)) {
                    //log
                }
            }
        
            //groups
            if (!dbm->getUserGroups(dbinterface.dbuser.user_id, dbinterface.dbgroups)) {
                //log
            }
       
            //groups info
            for (size_t i = 0; i < dbinterface.dbgroups.size(); i++) {
                //info
                if (!dbm->getGroupInfo(dbinterface.dbgroups[i].group_id, dbinterface.dbgroups[i])) {
                    //log
                }
                //members
                if (!dbm->getGroupMembers(dbinterface.dbgroups[i].group_id, dbinterface.dbgroups[i])) {
                }
                //member info
                for (size_t j = 0; j < dbinterface.dbgroups[i].members.size(); j++) {
                    //state
                    if (!dbm->getUserState(dbinterface.dbgroups[i].members[j].user_id, dbinterface.dbgroups[i].members[j])) {
                    }
                    //info
                    if (!dbm->getUserInfo(dbinterface.dbgroups[i].members[j].user_id, dbinterface.dbgroups[i].members[j])) {
                    }
                }
            }

            //talks
           
            if (!dbm->getUserTalks(dbinterface.dbuser.user_id, dbinterface.dbtalks)) {
                //log
            }
       
            //talks info
            for (size_t i = 0; i < dbinterface.dbtalks.size(); i++) {
                //info
                if (!dbm->getTalkInfo(dbinterface.dbtalks[i].talk_id, dbinterface.dbtalks[i])) {
                    //log
                }
                //members
                if (!dbm->getTalkMembers(dbinterface.dbtalks[i].talk_id, dbinterface.dbtalks[i])) {
                }
                //member info
                for (size_t j = 0; j < dbinterface.dbtalks[i].members.size(); j++) {
                    //state
                    if (!dbm->getUserState(dbinterface.dbtalks[i].members[j].user_id, dbinterface.dbtalks[i].members[j])) {
                    }
                    //info
                    if (!dbm->getUserInfo(dbinterface.dbtalks[i].members[j].user_id, dbinterface.dbtalks[i].members[j])) {
                    }
                }
            }
        }
        //TODO test
        msg->set_succ(succ);
        Value msgjson(objectValue);
        buildDBInterfaceJson(msgjson, dbinterface);
        string msgstr = getJsonStr(msgjson); 
        msg->set_msg(msgstr);
        msg->set_state(3);
    }
    else {
    }
    return 0;
}

int proc_exit_cmd(msg_t* msg, void *arg) {
    if (arg == NULL || msg == NULL) return -1;
    DBManager *dbm = (DBManager *)arg;
    int ret = 1;
    if (msg->user_id() > 0) {
        dbm->setUserState(msg->user_id(), 0);
        if (msg->state() == 2) { 
            msg->set_state(3);
            ret = 0;
        }
    } 
    return ret; 
}

int proc_text_cmd(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    DBManager *dbm = (DBManager *)arg;
    int ret = 1;
    if (msg->state() >= MAX_BASE_STATE) {
        if (msg->type() == TEXT_TYPE_FRIEND) {
            DBUser dbuser;
            if (dbm->getUser(msg->tuid(), dbuser)) {
                msg->set_user_id(dbuser.user_id);
                record_to_db(msg, dbm);        
            } 
            else {
                cerr<<"text message failed, uid:"<<msg->uid()<<" tuid:"<<msg->tuid()<<endl;
            }
        } 
        else {
            record_to_db(msg->tuser_id(), msg, dbm);
        }
    }
    else if (msg->state() == 2) {
        dbm->recordTextMessage(msg->user_id(), msg->uid(), msg->tuid(), msg->buf(), msg->buflen());
    }
    //not need retturn to ls
    return ret;
}

int proc_keepalive_cmd(msg_t *msg, void *arg) {
    return 0;
}

int proc_find_info(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    DBManager *dbm = (DBManager *)arg;
    if (msg->state() == 2) {
        int succ = 0;
        if (msg->type() == 1) {
            DBUser dbuser;
            if (!dbm->getUser(msg->tuid().c_str(), dbuser)) {
                succ = 1;
            }
            else {
                if (!dbm->getUserState(dbuser.user_id, dbuser) || !dbm->getUserInfo(dbuser.user_id, dbuser)) {
                    succ = 1;
                }
            }
            if (succ == 0) {
                Value json(objectValue);
                buildDBUserJson(json, dbuser);
                Value friendjson(objectValue);
                friendjson["friend"] = json;
                string msgstr = getJsonStr(friendjson);
                msg->set_msg(msgstr);
                cout<<"find:"<<msgstr<<endl;
            }
            msg->set_state(3);
            msg->set_succ(succ);
            return 0;
        }
        else if (msg->type() == 2) {
            //group
            vector <DBGroup> groups; 
            dbm->searchGroups(msg->tuid(), groups);
            Value json(objectValue);
            Value groupsarray(arrayValue);
            for (size_t i = 0; i < groups.size(); i++) {
                Value groupjson(objectValue);
                groupjson["group_id"] = groups[i].group_id;
                groupjson["name"] = groups[i].name;
                groupsarray.append(groupjson);
            }
            json["groupinfos"] = groupsarray;
            string msgstr = getJsonStr(json);
            msg->set_msg(msgstr);
            msg->set_state(3);
            msg->set_succ(0);
            return 0;
        }
    }
    return 1;
}

int proc_add_friend(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    DBManager *dbm = (DBManager *)arg;
    int ret = 1;
    if (msg->state() == 2) {
        //record
        DBUser dbuser;
        DBUser dbfriend; 
        Value json = parseJsonStr(msg->msg());
        string friendtype = "我的好友";
        get_str_member(json, "friend_type", friendtype);
        if (dbm->getUser(msg->uid(), dbuser) && dbm->getUser(msg->tuid(), dbfriend)) {
            if (!dbm->addFriend(dbuser.user_id, dbfriend.user_id, friendtype)) {
                msg->set_succ(2);
            }
            else if (!dbm->addFriend(dbfriend.user_id, dbuser.user_id, friendtype)) {
                msg->set_succ(2);
            }
            else {
                msg->set_succ(0); 
            }
        }
        else {
            msg->set_succ(2);
        }
        msg->set_state(3);
        ret = 0;
    }
    else if (msg->state() >= MAX_BASE_STATE) {
        //add record to db
        DBUser dbuser;
        if (dbm->getUser(msg->tuid(), dbuser)) {
            msg->set_user_id(dbuser.user_id);
            record_to_db(msg, dbm);        
        }
    }
    return ret;
}

//type: 1:team, 2:friend, 3:group_friend, 4:group, 5:talk
int proc_del_friend(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    DBManager *dbm = (DBManager *)arg;
    int ret = 1;
    int succ = 0;
    if (msg->state() == 2) {
        if (msg->type() == 1) {
        }
        else if (msg->type() == 2) {
            dbm->delFriend(msg->user_id(), msg->tuser_id());        
            dbm->delFriend(msg->tuser_id(), msg->user_id());        
        }
        else if (msg->type() == 3) {
        }
        else if (msg->type() == 4) {
            int member_type = dbm->getGroupMemberType(msg->tuser_id(), msg->user_id());
            if (member_type < 0) {
                succ = 1;
            }
            else if (member_type == GROUP_MEMBER_ADMIN) { //owner
                dbm->delGroupMember(msg->tuser_id());
            }
            else {
                dbm->delGroupMember(msg->tuser_id(), msg->user_id());
            }
            string tmp('t', member_type + 1);
            msg->set_buf(tmp.c_str(), member_type + 1);
        }
        else { succ = 100; }
        msg->set_state(3);
        ret = 0;
        msg->set_succ(succ);
    }
    else if (msg->state() >= MAX_BASE_STATE) {
        //record to db
        record_to_db(msg->tuser_id(), msg, dbm);
    }
    return ret;
}

static int proc_create_group(msg_t *msg, void *arg) {
    DBManager *dbm = (DBManager *)arg;
    if (msg->state() == 2) {
        int succ = 0;
        int t = (int) time(NULL); 
        int user_id = msg->user_id();
        string notice="null";
        string headurl = "null";
        DBGroup dbgroup;
        if (!dbm->createGroup(user_id, msg->tuid(), notice, headurl, t)) {
            succ = 1; 
        } 
        else if (!dbm->getGroupInfo(user_id, t, dbgroup)) {
            succ = 2;
        }
        else if (dbgroup.group_id <= 0){
            succ = 2;
        }
        else if (!dbm->addGroupMember(dbgroup.group_id, user_id, GROUP_MEMBER_ADMIN)) {
            //add member to group
            succ = 3;
        }
        else {
            dbm->getGroupMembers(dbgroup.group_id, dbgroup);
     	    for (size_t j = 0; j < dbgroup.members.size(); j++) {
                //state
                if (!dbm->getUserState(dbgroup.members[j].user_id, dbgroup.members[j])) {
                }
                //info
                if (!dbm->getUserInfo(dbgroup.members[j].user_id, dbgroup.members[j])) {
                }
            }
            succ = 0;
            Value json(objectValue);
            json["group"] = buildGroupJson(dbgroup);
            string strmsg = getJsonStr(json);
            msg->set_msg(strmsg);
        }
        msg->set_succ(succ);
        msg->set_state(3);
        return 0;
    }
    return 1; 
}

int proc_add_group_verify(msg_t *msg, void *arg) {
    int ret = 1;
    DBManager *dbm = (DBManager *)arg;
    if (msg->state() == 2) {
        int succ = 0;
        Value json = parseJsonStr(msg->msg());
        if (json.isObject() && json.isMember("group") && json["group"].isObject()) {
            int group_id = json["group"]["group_id"].asInt();
            DBGroup dbgroup;
            if (!dbm->getGroupInfo(group_id, dbgroup)) {
                succ = 1;
            }
            else {
                msg->set_tuser_id(dbgroup.admin_id);
            } 
        }
        else {
            succ = 1000;
        }
        msg->set_state(3);
        ret = 0;
    } 
    else if (msg->state() >= MAX_BASE_STATE) {
        record_to_db(msg->tuser_id(), msg, dbm); 
    }
    return ret;
}

int proc_add_group_reply(msg_t *msg, void *arg) {
    int ret = 1;
    DBManager *dbm = (DBManager *)arg;
    if (msg->state() == 2) {
        Value json = parseJsonStr(msg->msg());
        int succ = 0;
        if (!json.isObject() && !json.isMember("group") || !json["group"].isObject()) {
            LOG4CXX_DEBUG(logger_, "proc_add_group_reply msg has invalid friend"<<msg->msg());
            succ = 1;
        }
        else {
            DBGroup dbgroup;
            dbgroup.group_id = -1;
            get_int_member(json["group"], "group_id", dbgroup.group_id);
            if (dbgroup.group_id > 0) {
                dbm->addGroupMember(dbgroup.group_id, msg->user_id(), 0);
            } 
            dbm->getGroupInfo(dbgroup.group_id, dbgroup);
            dbm->getGroupMembers(dbgroup.group_id, dbgroup);
            //member info
            for (size_t j = 0; j < dbgroup.members.size(); j++) {
                //state
                if (!dbm->getUserState(dbgroup.members[j].user_id, dbgroup.members[j])) {
                }
                //info
                if (!dbm->getUserInfo(dbgroup.members[j].user_id, dbgroup.members[j])) {
                }
            }

            json["group"] = buildGroupJson(dbgroup);
        }
        string strmsg = getJsonStr(json);
        msg->set_msg(strmsg);
        msg->set_succ(succ);
        msg->set_state(3);
        ret = 0;
    }
    else if (msg->state() >= MAX_BASE_STATE) {
        record_to_db(msg->user_id(), msg, dbm);
    }
    return ret;
}

int proc_modify_group(msg_t *msg, void *arg) {
    DBManager *dbm = (DBManager *)arg;
    int ret = 1;
    if (msg->state() == 2) {
        string group_name = msg->tuid();
        if (dbm->modifyGroupName(msg->tuser_id(), msg->user_id(), group_name)) {
            msg->set_succ(0);
        }
        else {
            msg->set_succ(1);
        }
        msg->set_state(3);
        ret = 0;
    }
    return ret;
}
   
int proc_group_info(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    int ret = 1;
    if (msg->type() == GROUP_INFO_VERIFY) {
        ret = proc_add_group_verify(msg, arg);
    }
    else if (msg->type() == GROUP_INFO_REPLY) {
        ret = proc_add_group_reply(msg, arg);
    }
    else if (msg->type() == 2) {
    }
    else if (msg->type() == GROUP_INFO_CREATE) {
        ret = proc_create_group(msg, arg);
    }
    else if (msg->type() == GROUP_INFO_MODIFY) {
        ret = proc_modify_group(msg, arg);
    }
    return ret;
}

int proc_load_messages_cmd(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    DBManager *dbm = (DBManager *)arg;
    int ret = 1;
    vector <string> messages;
    if (dbm->getUserMessages(msg->user_id(), messages)) {
        for (size_t i = 0; i < messages.size(); i++) {
            msg_t *msg = new msg_t();
            const char* buf = messages[i].c_str();
            msg->unserialize((char*)buf);
            push_send_event(msg); 
        }
        //TODO
        dbm->deleteUserMessages(msg->user_id());
    } 
    cout<<"load_messages size:"<<messages.size()<<" user_id:"<<msg->user_id()<<endl;
    return ret;
}

int proc_report_cmd(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    DBManager *dbm = (DBManager *)arg;
    int ret = 1;
    DBUser dbuser;
    int succ;
    if (dbm->getUser(msg->tuid(), dbuser)) {
        //type : from_ip
        if (dbm->report(msg->user_id(), msg->type(), msg->tuid(), msg->buf(), msg->buflen())) { 
            succ = 0;
        }
        else succ = 1;
    }    
    else {
        succ = 2;
    }
    msg->set_succ(succ);
    msg->set_state(3);
    ret = 0;
    return ret;
}

int proc_setinfo_cmd(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    int ret = 1;
    if (msg->state() == 2) {
        DBManager *dbm = (DBManager *)arg;
        int succ = 1000;
        if (msg->type() == 0) { //set invited
            int invited = -1;
            Value json = parseJsonStr(msg->msg());
            if (json.isObject() && json.isMember("invited")) {
                invited = json["invited"].asInt();
            }
            else {
                succ = 1;
            }
            if (invited >=0 && dbm->setUserInvited(msg->user_id(), invited)) {
                succ = 0;
            }
            else succ = 2;
        }
        msg->set_state(3);
        msg->set_succ(succ);
        ret = 0;
    }
    return ret;
}


