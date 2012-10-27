#include "logic_proc.h"
#include "../net/net_func.h"
#include "../common/utils.h"

extern map <string, user_t *> idu_map;
extern map <int, struct conn_t *> fdc_map;
extern vector <struct conn_t *> dbserver_conns;

static LogicCmd logic_cmd[] = {
    {CMD_LOGIN, proc_login_cmd},
    {CMD_EXIT, proc_exit_cmd},
    {CMD_KA, proc_keepalive_cmd},
};

int proc_cmd(msg_t* msg, conn_t *conn) {
    int ret = -1;
    if (msg->cmd() >= 0 && (unsigned int)msg->cmd() < sizeof(logic_cmd)/sizeof(logic_cmd[0])) {
        ret = (*(logic_cmd[msg->cmd()].callback_))(msg, conn);
    }
    else {
        LOG4CXX_ERROR(logger_, "proc_cmd, err msg cmd:"<<msg->cmd());
    }
    return ret;
}

static int send_to_dbserver(msg_t *msg) {
    static int index = 0;
    int i = (index++) % dbserver_conns.size();
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
    idu_map.erase(user->uid);
    if (user->conn) {
        clean_conn(user->conn);   
    }
}

void send_user_exit(user_t* user) {
    msg_t msg;
    msg.set_cmd(CMD_EXIT);
    msg.set_uid(user->uid);
    msg.set_state(4);
    send_to_dbserver(&msg);
}

void send_keepalive(conn_t *conn, msg_t *msg) {
    //TODO
}


int proc_login_cmd (msg_t *msg, conn_t *conn) {
    if (msg->state() == 1) {
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter != idu_map.end()) {
            msg->set_succ(1);
            send_to_client(msg, conn);
            return 1;
        }
   
        user_t* user = new user_t();
        if (user == NULL) {
            LOG4CXX_ERROR(logger_, "proc_login_cmd, malloc memory failed, state:"<<msg->state()<<" uid:"<<msg->uid());
            return -1;
        }   
        user->uid = msg->uid();
        user->state = STATE_WAIT_LOGIN;
        user->conn = conn;
        conn->data.ptr = (void*)user;
        idu_map[user->uid] = user;    
          
        msg->set_state(2);
        send_to_dbserver(msg);

        conn->invalid_time = hl_timestamp() + 5*1000*1000;
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
            user->conn->invalid_time = hl_timestamp() + 5*1000*1000;
        }
        else {
            user->state = STATE_AUTH_FAILED;
	    user->conn->invalid_time = 0;
            user->conn->data.ptr = 0;
	}
    }
    else {
        //LOG4CXX_ERROR(logger_, "proc_login_cmd:"<<(*msg));
    }

    return 0;
}

int proc_exit_cmd(msg_t* msg, conn_t* conn) {
    if (msg->state() == 1) {
        user_t *user = (user_t *)conn->data.ptr;
        if (user == NULL || user->state != STATE_LOGINED) {
            return -1;
        }
        user->state = STATE_EXIT;
        msg->set_state(2);
        send_to_dbserver(msg);
     
        conn->invalid_time = hl_timestamp() + 5*1000*1000;
    }
    else if (msg->state() == 3) {
        map <string, user_t *>::iterator uiter = idu_map.find(msg->uid());
        if (uiter == idu_map.end()) {
            LOG4CXX_ERROR(logger_, "proc_exit_cmd not found user, state:"<<msg->state()<<" uid:"<<msg->uid());
            return -2;
        }
        user_t *user = uiter->second;
        user->conn->invalid_time = 0;
        if (user->conn) {
            send_to_client(msg, user->conn);
        }
    }
    else {
        //LOG4CXX_ERROR(logger_, "proc_exit_cmd:"<<(*msg));
    }

    return 0; 
}

int proc_keepalive_cmd(msg_t *msg, conn_t *conn) {
    if (conn->mark == CONN_CLIENT) {
        user_t *user = (user_t *)conn->data.ptr;
        if (conn->invalid || user == NULL || user->state != STATE_LOGINED) {
            return 1;
        }
        //TODO
    }
    conn->invalid_time = hl_timestamp() + 5*1000*1000;
    return 0;
}



