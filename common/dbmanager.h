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

private:

    Connection conn_;

    string host_;
    int    port_;
    string database_;
    string user_;
    string pwd_;
};

#endif //__DB_MANAGER_H__

