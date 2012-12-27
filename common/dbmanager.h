#ifndef __DB_MANAGER_H__
#define __DB_MANAGER_H__

#include "mysql++.h"

#include <string>

using namespace mysqlpp;
using namespace std;

class DBManager {
    
public:
    DBManager(){}
    DBManager(const char *host, int port, const char *database, const char *user, const char *pwd);
    ~DBManager();

    int init(const char *host, int port, const char *database, const char *user, const char *pwd);
    bool connectMysql(Connection &conn);
    bool connectMysql();
    void closeMysql();

    int getStoreData(const char *sql, StoreQueryResult &result);
    int getUser(const char *name, StoreQueryResult &result);
    int getUserInfo(int user_id, StoreQueryResult &result) ;
    int getUserState(int user_id, StoreQueryResult &result);
    int getFriends(int user_id, StoreQueryResult &result);
    int getGroupInfo(int group_id, StoreQueryResult &result);
    int getGroupMembers(int group_id, StoreQueryResult &result);
    int getUserGroups(int user_id, StoreQueryResult &result);
    int getTalkInfo(int talk_id, StoreQueryResult &result);
    int getTalkMembers(int talk_id, StoreQueryResult &result);
    int getUserTalks(int user_id, StoreQueryResult &result);

private:

    Connection conn_;

    string host_;
    int    port_;
    string database_;
    string user_;
    string pwd_;
};

#endif //__DB_MANAGER_H__

