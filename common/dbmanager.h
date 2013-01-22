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
    bool checkConnection();
   
    int execSql(const char *sql);
    int execSql(const string &sql);
    int getStoreData(const char *sql, StoreQueryResult &result);
    int getStoreData(const char *sql, UseQueryResult &result);
    int getStoreData(string &sql, StoreQueryResult &result);

    //int getUser(const char *name, DBUser &dbu);
    int getUser(const string &name, DBUser &dbu);
    int getUserInfo(int user_id, DBUser &dbu);
    int getUserState(int user_id, DBUser &dbu) ;
    int setUserState(int user_id, int state);

    int getFriends(int user_id, vector <DBFriend> &dbfriends);
    int addFriend(int user_id, int friend_id, string &type);
    int modifyFriend(int user_id, int friend_id, string &newtype);
    int delFriend(int user_id, int friend_id);

    int createGroup(int user_id, string group_name, string &notice, string &headurl, int time);
    int getGroupInfo(int user_id, int time, DBGroup &dbgroup); 
    int getGroupInfo(int group_id, DBGroup &dbgroup);
    int getGroupMembers(int group_id, DBGroup &dbgroup);
    int getUserGroups(int user_id, vector <DBGroup> &groups);

    int getTalkInfo(int talk_id, DBTalks &dbtalks);
    int getTalkMembers(int talk_id, DBTalks &dbtalks); 
    int getUserTalks(int user_id, vector <DBTalks> &talks);

    int getUserMessages(int user_id, vector <string> &messages);
    int setUserMessages(int user_id, const string &messages);
    int deleteUserMessages(int user_id);

private:

    Connection conn_;

    string host_;
    int    port_;
    string database_;
    string user_;
    string pwd_;
};

#endif //__DB_MANAGER_H__

