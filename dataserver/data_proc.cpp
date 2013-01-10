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

extern AppConfig config_;
extern log4cxx::LoggerPtr logger_;
static LogicCmd logic_cmd[] = {
    {CMD_RESERVE, NULL},
    {CMD_LOGIN, proc_login_cmd},
    {CMD_GET_FRIEND, NULL},
    {CMD_GET_GROUPINFO, NULL},
    {CMD_EXIT, proc_exit_cmd},
    {CMD_TEXT, NULL},
    {CMD_TRANS_FILE, NULL},
    {CMD_SHARE_FILE, NULL},
    {CMD_TRANS_VIDEO, NULL},
    {CMD_MODIFY_INFO, NULL},
    {CMD_MODIFY_GROUP_INFO, NULL},
    {CMD_MODIFY_FRIEND, NULL},
    {CMD_GETALL_USERS, NULL},
    {CMD_FIND_USER, proc_find_info},
    {CMD_ADD_FRIEND, proc_add_friend},
    {CMD_DEL_FRIEND, NULL},
    {CMD_KA, proc_keepalive_cmd},
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
                if (!dbm->getUserState(dbinterface.dbfriends[i].dbuser.user_id, dbinterface.dbfriends[i].dbuser)) {
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
                    if (!dbm->getUserState(dbinterface.dbgroups[i].members[i].user_id, dbinterface.dbgroups[i].members[i])) {
                    }
                    //info
                    if (!dbm->getUserInfo(dbinterface.dbgroups[i].members[i].user_id, dbinterface.dbgroups[i].members[i])) {
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
                    if (!dbm->getUserState(dbinterface.dbtalks[i].members[i].user_id, dbinterface.dbtalks[i].members[i])) {
                    }
                    //info
                    if (!dbm->getUserInfo(dbinterface.dbtalks[i].members[i].user_id, dbinterface.dbtalks[i].members[i])) {
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

int proc_keepalive_cmd(msg_t *msg, void *arg) {
    return 0;
}

int proc_find_info(msg_t *msg, void *arg) {
    assert(msg != NULL && arg != NULL);
    DBManager *dbm = (DBManager *)arg;
    if (msg->state() == 2) {
        int succ = 0;
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
            string msgstr = getJsonStr(json);
            msg->set_msg(msgstr);
        }
        msg->set_state(3);
        msg->set_succ(succ);
        return 0;
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
        string friendtype = "online";
        if (dbm->getUser(msg->uid(), dbuser) && dbm->getUser(msg->tuid(), dbfriend)) {
            dbm->addFriend(dbuser.user_id, dbfriend.user_id, friendtype);
            dbm->addFriend(dbfriend.user_id, dbuser.user_id, friendtype);
            msg->set_succ(0); 
        }
        else {
            msg->set_succ(2);
        }
        ret = 0;
    }
    else if (msg->state() == 4) {
        //add record to db
    }
    return ret;
}


