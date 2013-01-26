#include "logic_proc.h"
#include "../net/net_func.h"
#include "../common/utils.h"
#include "../common/def.h"
#include "../common/dbstruct.h"
#include "../common/msginterface.h"
#include "../common/heap.h"

#include <assert.h>

extern map <string, user_t *> idu_map;
extern map <int, user_t *> user_map;
extern map <int, struct conn_t *> fdc_map;
extern vector <struct conn_t *> dbserver_conns;
extern map <int, vector <int> > user_groups_;
extern map <int, vector <int> > user_talks_;
extern heap <conninfo, greater<conninfo> > conn_timeout;

static LogicCmd logic_cmd[] = {
    {CMD_RESERVE, NULL},   //0
    {CMD_LOGIN, proc_login_cmd}, //1
    {CMD_GET_FRIEND, NULL},  //2
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
    {CMD_GROUP_INFO, NULL}, //16
    {CMD_TALK_INFO, NULL}, //17
    {CMD_KA, proc_keepalive_cmd}, //18
    {CMD_LOAD_MESSAGES, proc_load_messages}, //19
};

extern AppConfig config_;
extern log4cxx::LoggerPtr logger_;

int proc_cmd(msg_t* msg, conn_t *conn) {
    int ret = -1;
    if (msg->cmd() > 0 && (unsigned int)msg->cmd() < sizeof(logic_cmd)/sizeof(logic_cmd[0])) {
        ret = (*(logic_cmd[msg->cmd()].callback_))(msg, conn);
    }
    else {
        LOG4CXX_ERROR(logger_, "proc_cmd, err msg cmd:"<<msg->cmd());
    }
    return ret;
}

static int send_to_dbserver(msg_t *msg) {
    if (dbserver_conns.size() == 0) {
        LOG4CXX_ERROR(logger_, "not has dbserver conns");
        return 0;
    }
    static int index = 0;
    int i = (index++) % dbserver_conns.size();
    LOG4CXX_DEBUG(logger_, "send_to_dbserver, uid:"<<msg->uid()<<" tuid:"<<msg->tuid()<<" i:"<<i);
    return send_to_client(msg, dbserver_conns[i]);
}

void clean_conn(conn_t* conn) {
    fdc_map.erase(conn->fd);
    if (conn->mark == CONN_DB_SERVER) {
        for (size_t i = 0; i < dbserver_conns.size(); i++) {
            if (dbserver_conns[i] == conn) {
                dbserver_conns.erase(dbserver_conns.begin() + i);
            }
        }
    }
    destroy_conn(conn);
}

void clean_user(user_t* user) {
    if (user->id > 0) {
        user_map.erase(user->id);
    }
    idu_map.erase(user->uid);
    if (user->conn) {
        clean_conn(user->conn);   
    }
    delete user;
}

void send_user_exit(user_t* user) {
    msg_t msg;
    msg.set_user_id(user->id);
    msg.set_cmd(CMD_EXIT);
    msg.set_uid(user->uid);
    msg.set_state(4);
    send_to_dbserver(&msg);
}

user_t *get_login_user(string &uid) {
    return NULL; 
}

void send_load_messages(int user_id) {
    msg_t msg;
    msg.set_user_id(user_id);
    msg.set_cmd(CMD_LOAD_MESSAGES);
    send_to_dbserver(&msg);
}

void send_keepalive(conn_t *conn, msg_t *msg) {
    //TODO
}

static int user_del_friend(user_t *user, int friend_id) {
    assert(user != NULL);
    for (vector <int>::iterator viter = user->friend_ids.begin();
        viter != user->friend_ids.end(); viter++) {
        if (friend_id == *viter) {
            user->friend_ids.erase(viter);
            break;
        }
    }
    return 0;
}

int proc_login_cmd (msg_t *msg, conn_t *conn) {
    if (msg->state() == 1) {
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter != idu_map.end()) {
            msg->set_succ(1);
            send_to_client(msg, conn);
            LOG4CXX_DEBUG(logger_, "user already login in, uid<<"<<msg->uid());
            return 1;
        }
   
        user_t* user = new user_t();
        user->uid = msg->uid();
        user->state = STATE_WAIT_LOGIN;
        user->conn = conn;
        conn->ptr = (void*)user;
        idu_map[user->uid] = user;    
          
        msg->set_state(2);
        send_to_dbserver(msg);

        conn->invalid_time = hl_timestamp() + CONN_INVALID_TIME;
        conn_timeout.sort();
    }
    else if (msg->state() == 3) {
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter == idu_map.end()) {
            LOG4CXX_ERROR(logger_, "proc_login_cmd, not found user, state:"<<msg->state()<<" uid:"<<msg->uid());
            return -2;
        }
        user_t *user = uiter->second;
        send_to_client(msg, user->conn); 

        if (msg->succ() == 0) {
            user->state = STATE_LOGINED;
            user->conn->invalid_time = hl_timestamp() + CONN_INVALID_TIME;
            conn_timeout.sort();
            //parse dbinterface msg
            //TODO
            Value json =  parseJsonStr(msg->msg());
            DBInterface dbinterface;
            parse_dbinterface(json, dbinterface); 
            //get user id, get user firends, get_groups get friends
            user->id = dbinterface.dbuser.user_id;
            msg->set_user_id(user->id);
            for (size_t i = 0; i < dbinterface.dbfriends.size(); i++) {
                user->friend_ids.push_back(dbinterface.dbfriends[i].dbuser.user_id); 
            } 
            for (size_t i = 0; i < dbinterface.dbgroups.size(); i++) {
                user->group_ids.push_back(dbinterface.dbgroups[i].group_id);
                //update group
                vector <int> &members = user_groups_[dbinterface.dbgroups[i].group_id];
                members.clear();
                for (size_t j = 0; j < dbinterface.dbgroups[i].members.size(); j++) {
                    members.push_back(dbinterface.dbgroups[i].members[i].user_id);
                }
            }
            for (size_t i = 0; i < dbinterface.dbtalks.size(); i++) {
                user->talk_ids.push_back(dbinterface.dbtalks[i].talk_id);
                //update talks
                vector <int> &members = user_talks_[dbinterface.dbtalks[i].talk_id];
                members.clear();
                for (size_t j = 0; j < dbinterface.dbtalks[i].members.size(); j++) {
                    members.push_back(dbinterface.dbtalks[i].members[i].user_id);
                }
            }
            //send_load_messages(user->id);
            
            user_map[user->id] = user;
        }
        else {
            user->state = STATE_AUTH_FAILED;
	    user->conn->invalid_time = 0;
            conn_timeout.sort();
	}
        LOG4CXX_INFO(logger_, "user login, succ:"<<msg->succ()<<" uid:"<<user->uid<<" id:"<<user->id<<" state:"<<user->state);
    }
    else {
        LOG4CXX_ERROR(logger_, "proc_login_cmd invalid, state;"<<msg->state());
    }

    return 0;
}

int proc_exit_cmd(msg_t* msg, conn_t* conn) {
    if (msg->state() == 1) {
        user_t *user = (user_t *)conn->ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            return -1;
        }
        user->state = STATE_EXIT;
        msg->set_state(2);
        send_to_dbserver(msg);
     
        conn->invalid_time = hl_timestamp() + CONN_INVALID_TIME;
        conn_timeout.sort();
    }
    else if (msg->state() == 3) {
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter == idu_map.end()) {
            LOG4CXX_ERROR(logger_, "proc_exit_cmd not found user, state:"<<msg->state()<<" uid:"<<msg->uid());
            return -2;
        }
        user_t *user = uiter->second;
        user->conn->invalid_time = 0;
        conn_timeout.sort();
        send_to_client(msg, user->conn);
        LOG4CXX_INFO(logger_, "user exit, uid:"<<user->uid<<" id:"<<user->id);
    }
    else {
        LOG4CXX_ERROR(logger_, "proc_exit_cmd invalid, state:"<<msg->state());
    }

    return 0; 
}

int proc_keepalive_cmd(msg_t *msg, conn_t *conn) {
    if (conn->mark == CONN_CLIENT) {
        user_t *user = (user_t *)conn->ptr;
        if (conn->invalid || user == NULL || user->state != STATE_LOGINED) {
            return 1;
        }
        //TODO
    	send_to_client(msg, conn);
    }
    conn->invalid_time = hl_timestamp() + CONN_INVALID_TIME;
    conn_timeout.sort();
    return 0;
}

int proc_text_cmd(msg_t *msg, conn_t *conn) {
    LOG4CXX_DEBUG(logger_, "send text, uid:"<<msg->uid()<<" tuid:"<<msg->tuid());
    if (msg->state() < MAX_BASE_STATE) {
        user_t *user = (user_t *)conn->ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            LOG4CXX_DEBUG(logger_, "text, user not logined, uid:"<<msg->uid());
            return -1;
        }
        msg->set_user_id(user->id);
    }
    user_t *tuser = NULL;
    map <string, user_t *>::iterator uiter = idu_map.find(msg->tuid());
    if (uiter != idu_map.end()) {
        tuser = uiter->second;
    }
    if (tuser == NULL || tuser->state != STATE_LOGINED) {
        //TODO
        //msg message
        msg->set_state(msg->state()%MAX_BASE_STATE + MAX_BASE_STATE);
        LOG4CXX_DEBUG(logger_, "text, uid:"<<msg->uid()<<" tuid:"<<msg->tuid());
        send_to_dbserver(msg);
    }
    else {
        send_to_client(msg, tuser->conn);
        msg->set_state(2);
        send_to_dbserver(msg);
    }
    return 0;
}

int proc_find_info(msg_t *msg, conn_t *conn) {
    int ret = 0;
    LOG4CXX_DEBUG(logger_, "find user, uid:"<<msg->uid()<<" tuser:"<<msg->tuid()<<" state:"<<msg->state());
    if (msg->state() == 1) {
        user_t *user = (user_t *)conn->ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            LOG4CXX_DEBUG(logger_, "user:["<<msg->uid()<<"] not logined, "<<(user==NULL?"user is null":""));
            return -1;
        }
        msg->set_state(2);
        send_to_dbserver(msg);         
    }
    else if (msg->state() == 3) {
        user_t *tuser = NULL;
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter != idu_map.end()) {
            tuser = uiter->second;
        } 
        if (tuser != NULL && tuser->conn) {
            send_to_client(msg, tuser->conn);
        }
        else {
             ret = -1;
            //TODO
        }
    }
    return ret;
}

int proc_add_friend_message(msg_t *msg) {
    int ret = 0;
    if (msg->state() < MAX_BASE_STATE) {
        return -1;
    }
    user_t *tuser = NULL;
    map <string, user_t *>::iterator uiter = idu_map.find(msg->tuid());
    if (uiter != idu_map.end()) {
        tuser = uiter->second;
    }
    if (tuser != NULL) {
        if (tuser->conn) {
            send_to_client(msg, tuser->conn);
        }
        else {
            LOG4CXX_DEBUG(logger_, "add friend message proc failed, tuser.conn isnull, tuid:"<<msg->tuid());   
        }
    }
    if (msg->succ() == 0 && msg->type() == 1) {
        //record to db
        LOG4CXX_INFO(logger_, "user:["<<msg->uid()<<"] add friend:["<<msg->tuid()<<"], record to db");
        msg->set_state(2);
        send_to_dbserver(msg);
    }
    return ret;
}

int proc_add_friend(msg_t *msg, conn_t *conn) {
    int ret = 0;
    //type: 0 发送, 1:验证
    LOG4CXX_DEBUG(logger_, "add friend, uid:"<<msg->uid()<<" tuid:"<<msg->tuid()<<" state:"<<msg->state()<<" succ:"<<msg->succ());
    if (msg->state() >= MAX_BASE_STATE) {
        return proc_add_friend_message(msg);
    }
    if (msg->state() == 1) {
        if (msg->uid() == msg->tuid()) {
            LOG4CXX_DEBUG(logger_, "user can not add self, uid:"<<msg->uid());
            return -1;
        }
        user_t *user = (user_t *)conn->ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            LOG4CXX_DEBUG(logger_, "user not logined, uid:"<<msg->uid());
            return -1;
        }
        
        if (msg->type() == 0) {
            user_t *tuser = NULL;
            map <string, user_t *>::iterator uiter = idu_map.find(msg->tuid());
            if (uiter != idu_map.end()) {
                tuser = uiter->second;
            }
            if (tuser != NULL) {
                if (find(user->friend_ids.begin(), user->friend_ids.end(), tuser->id) != user->friend_ids.end()) {
                    LOG4CXX_DEBUG(logger_, "user already had friend, uid:"<<user->uid<<" tuid:"<<tuser->uid);
                    return -1;
                }
                if (tuser->conn) {
                    send_to_client(msg, tuser->conn);
                }
                else {
                    msg->set_state(4 + MAX_BASE_STATE);
                    send_to_dbserver(msg);
                }
            }
            else {
                //record to db //liuyan
                LOG4CXX_INFO(logger_, "user:["<<msg->uid()<<"] msg to tuser:["<<msg->tuid()<<"]");
                msg->set_state(4 + MAX_BASE_STATE);
                send_to_dbserver(msg);
            }
        }
        else if (msg->type() == 1) {
            user_t *tuser = NULL;
            map <string, user_t *>::iterator uiter = idu_map.find(msg->tuid());
            if (uiter != idu_map.end()) {
                tuser = uiter->second;
            }
            if (msg->succ() == 0) {
                //record to db
                LOG4CXX_INFO(logger_, "user:["<<msg->uid()<<"] add friend:["<<msg->tuid()<<"], record to db");
                msg->set_state(2);
                send_to_dbserver(msg);
            }
            else if (user != NULL && tuser->conn) {
                send_to_client(msg, tuser->conn);
            }
            else {
                //log
            }
        }
        else {
            ret = -1;
        }
    }
    else if (msg->state() == 3) {
        user_t *user = NULL;
        user_t *tuser = NULL;
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter != idu_map.end()) {
             user = uiter->second;
        } 
        uiter = idu_map.find(msg->tuid());
        if (uiter != idu_map.end()) {
             tuser = uiter->second;
        }
        if (user != NULL && tuser != NULL && msg->succ() == 0) {
            user->friend_ids.push_back(tuser->id);
            tuser->friend_ids.push_back(user->id);  
        }
        if (tuser != NULL && tuser->conn) {
            send_to_client(msg, tuser->conn);
        }
    }
    return ret;
}

int proc_del_friend(msg_t *msg, conn_t *conn) {
    LOG4CXX_DEBUG(logger_, "recv del_friend_cmd, uid:"<<msg->uid()<<" tuid:"<<msg->tuid()<<" state:"<<msg->state()<<" succ:"<<msg->succ());
    int ret = 0;
    if (msg->state() == 1) {
        if (msg->uid() == msg->tuid()) {
            return -1;
        }
        user_t *user = (user_t *)conn->ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            LOG4CXX_DEBUG(logger_, "del friend failed, user is not login"<<(user==NULL?"user is null":""));
            return -1;
        }
        msg->set_user_id(user->id);
        msg->set_state(2);
        send_to_dbserver(msg);
    }
    else if (msg->state() == 3) {
        user_t *user = NULL;
        user_t *tuser = NULL;
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter != idu_map.end()) {
            user = uiter->second;
        }
        if (user != NULL) {
            if  (user->conn) {
                send_to_client(msg, user->conn); 
            }
            if (msg->succ() == 0) {
                Value json = parseJsonStr(msg->msg());
                int friend_id;
                if (get_int_member(json, "id", friend_id)) {
                    user_del_friend(user, friend_id);
                }
            }
        }
        uiter = idu_map.find(msg->tuid());
        if (uiter != idu_map.end()) {
            tuser = uiter->second;
        }
        if (tuser != NULL) {
            if (tuser->conn) {
                send_to_client(msg, tuser->conn);
            }
            if (msg->succ() == 0) {
                user_del_friend(tuser, msg->user_id());
            }      
        }         
    }
    return ret;
}

static int proc_create_group(msg_t *msg, conn_t *conn) {
    if (msg->state() == 1) {
        user_t *user = (user_t *)conn->ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            return -1;
        }
        //TODO config
        if (user->group_ids.size() >= 2) {
            return -1;
        }
        msg->set_user_id(user->id);
        msg->set_state(2);
        send_to_dbserver(msg);
    }    
    else if (msg->state() == 3) {
        map <string, user_t *>::iterator iter = idu_map.find(msg->uid());
        if (iter == idu_map.end() || iter->second == NULL) {
            return -1; 
        }
        user_t *user = iter->second;
        if (user->conn) {
            send_to_client(msg, user->conn);
        }
        if (msg->succ() == 0) {
            Value json = parseJsonStr(msg->msg());
            DBGroup group;
            if (check_arr_member(json, "group")) {
                parse_group(json["group"], group);
            }        
            if (group.group_id > 0) {
                user->group_ids.push_back(group.group_id);
                vector <int>& group_members = user_groups_[group.group_id];
                group_members.push_back(user->id); 
            }
        }
    }
    return 0;
}

int proc_group_info(msg_t *msg, conn_t *conn) {
    int ret = 0;
    assert(msg != NULL && conn != NULL);
    if (msg->type() == 0) {
    } 
    else if (msg->type() == 1) {

    }
    else if (msg->type() == 2) {
    }
    else if (msg->type() == 3) {
        ret = proc_create_group(msg, conn);
    }
    return ret;
}



int proc_load_messages(msg_t *msg, conn_t *conn) {
    int ret = 0;
    LOG4CXX_DEBUG(logger_, "load message, uid:"<<msg->uid());
    if (msg->state() == 1) {
        user_t *user = (user_t *)conn->ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            return -1;
        }
        msg->set_user_id(user->id);
        msg->set_state(2);
        send_to_dbserver(msg);
    }    
    return ret;
}

// 

