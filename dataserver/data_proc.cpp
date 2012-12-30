#include "data_proc.h"
#include "../net/net_func.h"
#include "../common/utils.h"
#include "../event/event_queue.h"
#include "../common/md5.h"
#include "../common/dbmanager.h"
#include "../json/json_util.h"
#include "../common/msginterface.h"

extern AppConfig config_;
extern log4cxx::LoggerPtr logger_;

static LogicCmd logic_cmd[] = {
    {CMD_RESERVE, NULL},
    {CMD_LOGIN, proc_login_cmd},
    {CMD_EXIT, proc_exit_cmd},
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

/*
void send_keepalive(conn_t *conn, msg_t *msg) {
    //TODO
}
*/

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
    return 0; 
}

int proc_keepalive_cmd(msg_t *msg, void *arg) {
    return 0;
}



