#include "dbmanager.h"
//g++ -g -DTEST_ $(mysql_config --cflags) dbmanager.cpp $(mysql_config --libs) libmysqlpp.so 
//
DBManager::DBManager(const char *host, int port, const char *database, const char *user, const char *pwd) {
    init(host, port, database, user, pwd);
}

int DBManager::init(const char *host, int port, const char *database, const char *user, const char *pwd) {
    host_ = host;
    port_ = port;
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

int DBManager::getStoreData(const char *sql, StoreQueryResult &result) {
    Query query = conn_.query();
    query<<sql;
    result = query.store();
    if (result) {
        return 1;
    }
    return 0;
}

int DBManager::getUser(const char *name, StoreQueryResult &result) {
    //TODO
    string sql = "select `name`,`pwd` from `test_admin`";
    return getStoreData(sql.c_str(), result);
}

int DBManager::getUserInfo(int user_id, StoreQueryResult &result) {
    string sql;
    //TODO call produce
    return getStoreData(sql.c_str(), result);
}

int DBManager::getUserState(int user_id, StoreQueryResult &result) {
    static char sqlu[256];
    sprintf(sqlu, "select `state`,`invited` from user_state where `user_id`='%d'\0", user_id);
    return getStoreData(sqlu, result);
}

int DBManager::getFriends(int user_id, StoreQueryResult &result) {
    static char sqlf[256];
    sprintf(sqlf,"select `friend_id`,`type` from `user_friend` where `user_id`='%d'\0", user_id);
    return getStoreData(sqlf, result); 
}

int DBManager::getGroupInfo(int group_id, StoreQueryResult &result) {
    static char sqlgi[256]; 
    sprintf(sqlgi, "select `name`,`notice`,`headurl` from `group_info` where `group_id`=`%d`\0", group_id);
    return getStoreData(sqlgi, result);
}

int DBManager::getGroupMembers(int group_id, StoreQueryResult &result) {
    static char sqlgm[256]; 
    sprintf(sqlgm, "select `member_id` from `group_members` where `group_id`='%d'\0", group_id);
    return getStoreData(sqlgm, result);
}

int DBManager::getUserGroups(int user_id, StoreQueryResult &result) {
    static char sqlug[256];
    sprintf(sqlug, "select `group_id` from `group_members` where `member_id`='%d'\0", user_id);
    return getStoreData(sqlug, result);
}

int DBManager::getTalkInfo(int talk_id, StoreQueryResult &result) {
    static char sqlti[256];
    sprintf(sqlti, "select `name`,`headurl` from `talk_info` where `talk_id`='%d'\0", talk_id);
    return getStoreData(sqlti, result);
}

int DBManager::getTalkMembers(int talk_id, StoreQueryResult &result) {
    static char sqltm[256];
    sprintf(sqltm, "select `member_id` from `talk_members` where `talk_id`='%d'\0", talk_id);
    return getStoreData(sqltm, result);
}

int DBManager::getUserTalks(int user_id, StoreQueryResult &result) {
    static char sqlut[256];
    sprintf(sqlut, "select `talk_id` from `talk_members` where `member_id`='%d'\0", user_id);
    return getStoreData(sqlut, result);
}

#ifdef TEST_
int main(int argc, char **argv) {
    DBManager dbm;
    dbm.init("192.168.3.99", 3306, argv[1], argv[2], argv[3]);
    if (!dbm.connectMysql()) {
        cout<<"connect failed"<<endl;
        return 1;
    }
    StoreQueryResult res;
    if (dbm.getUser("test", res)) {
        for (int i = 0; i < res.num_rows(); i++) {
            cout<<res[i][0]<<" "<<res[i][1]<<endl;
        }
    }
    else {
        cout<<"get User failed"<<endl;
        return 1;
    }
    return 0;
}

#endif

