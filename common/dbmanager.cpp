#include "dbmanager.h"

DBManager::DBManager(const char *host, int port, const char *database, const char *user, const char *pwd) {
    init(host, port, database, user, pwd);
}

int DBManager::init(const char *host, int port, const char *database, const char *user, const char *pwd) {
    host_ = host;
    port = port;
    database_ = database;
    user_ = user;
    pwd_ = pwd;
    return 0;
}

DBManager::~DBManager() {
    closeMysql();
}

void DBManager::closeMysql() {
    if (conn_.connected()) {
        conn_.disconnect();
    }
}

bool DBManager::connectMysql() {
    return connectMysql(conn_);
}

bool DBManager::connectMysql(Connection &conn) {
    if (conn_.connected()) {
        return true;
    }
    if (conn_.connect(database_.c_str(), host_.c_str(), user_.c_str(), pwd_.c_str(), port_)){
        Query q = conn.query("set names utf8");
        q.execute();
        return true;
    }
    else {
        cerr<<"connect to data failed\n";
        return false;   
    }
}

