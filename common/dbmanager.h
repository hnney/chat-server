#ifndef __DB_MANAGER_H__
#define __DB_MANAGER_H__

#include "mysql++.h"
#include "dbstruct.h"
#include "md5.h"

#include <string>
#include <vector>

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
    int getStoreData(const char *sql, UseQueryResult &result);
    int getUser(const char *name, DBUser &dbu);
    int getUserInfo(int user_id, DBUser &dbu);
    int getUserState(int user_id, DBUser &dbu) ;
    int getFriends(int user_id, vector <DBFriend> &dbfriends);
    int getGroupInfo(int group_id, StoreQueryResult &result);
    int getGroupInfo(int group_id, DBGroup &dbgroup);
    int getGroupMembers(int group_id, DBGroup &dbgroup);
    int getUserGroups(int user_id, vector <DBGroup> &groups);
    int getTalkInfo(int talk_id, DBTalks &dbtalks);
    int getTalkMembers(int talk_id, DBTalks &dbtalks); 
    int getUserTalks(int user_id, vector <DBTalks> &talks);

private:

    Connection conn_;

    string host_;
    int    port_;
    string database_;
    string user_;
    string pwd_;
};

#endif //__DB_MANAGER_H__

