#include "dbmanager.h"
//g++ -g -DTEST_ $(mysql_config --cflags) dbmanager.cpp $(mysql_config --libs) libmysqlpp.so 
//

typedef vector<int> IntVectorType;

static void print_row(IntVectorType& widths, Row& row)
{
    cout << "  |" << setfill(' ');
    for (size_t i = 0; i < row.size(); ++i) {
        cout << " " << setw(widths.at(i)) << row[i] << " |";
    }
    cout << endl;
}

static void
print_header(IntVectorType& widths, StoreQueryResult& res)
{
    cout << "  |" << setfill(' ');
    for (size_t i = 0; i < res.field_names()->size(); i++) {
        cout << " " << setw(widths.at(i)) << res.field_name(i) << " |";
    }
    cout << endl;
}

static void
print_row_separator(IntVectorType& widths)
{
    cout << "  +" << setfill('-');
    for (size_t i = 0; i < widths.size(); i++) {
        cout << "-" << setw(widths.at(i)) << '-' << "-+";
    }
    cout << endl;
}
static void
print_result(StoreQueryResult& res, int index)
{
    // Show how many rows are in result, if any
    StoreQueryResult::size_type num_results = res.size();
    if (res && (num_results > 0)) {
        cout << "Result set " << index << " has " << num_results <<
                " row" << (num_results == 1 ? "" : "s") << ':' << endl;
    }   
    else {
        cout << "Result set " << index << " is empty." << endl;
        return;
    }   

    // Figure out the widths of the result set's columns
    IntVectorType widths;
    int size = res.num_fields();
    for (int i = 0; i < size; i++) {
        widths.push_back(max(
                res.field(i).max_length(),
                res.field_name(i).size()));
    }   

    // Print result set header
    print_row_separator(widths);
    print_header(widths, res);
    print_row_separator(widths);

    // Display the result set contents
    for (StoreQueryResult::size_type i = 0; i < num_results; ++i) {
        print_row(widths, res[i]);
    }   

    // Print result set footer
    print_row_separator(widths);
}

static void
print_multiple_results(Query& query)
{
    // Execute query and print all result sets
    StoreQueryResult res = query.store();
    //print_result(res, 0); 
    for (int i = 1; query.more_results(); ++i) {
        res = query.store_next();
        print_result(res, i); 
    }
}

DBManager::DBManager(const char *host, int port, const char *database, const char *user, const char *pwd)
:conn_(false) {
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
    if (conn.connected()) {
        return true;
    }
    conn.set_option(new MultiStatementsOption(CLIENT_MULTI_STATEMENTS));
    if (conn.connect(database_.c_str(), host_.c_str(), user_.c_str(), pwd_.c_str(), port_)){
	conn.select_db("hdm0210494_db");
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
    int ret = 0;
    Query query = conn_.query();
    query<<sql;
    result = query.store();
    if (result) {
        ret = 1;
    }
    StoreQueryResult res;
    for (int i = 1; query.more_results(); ++i) {
        res = query.store_next();
    }
    return ret;
}

int DBManager::getStoreData(const char *sql, UseQueryResult &result) {
    int ret = 0;
    Query query = conn_.query();
    query<<sql;
    result = query.use();
    if (result) {
        ret = 1;
    }
    //TODO
    return ret;
}

int DBManager::getUser(const char *name, DBUser &dbu) {
    char sql[128];
    sprintf(sql, "call user_login('%s')", name);
    StoreQueryResult result;
    int ret = 0;
    if (getStoreData(sql, result)) {
        cout<<"rows:"<<result.num_rows()<<endl;
        if (result.num_rows() > 0 && result[0].size() >= 4) {
            dbu.user_id = atoi(result[0][0].c_str());
            dbu.user_name = result[0][1].c_str();
            dbu.user_pwd = result[0][2].c_str();
            dbu.user_pwd_key = result[0][3].c_str();
            ret = 1;
        }
    }
    return ret;
}

int DBManager::getUserInfo(int user_id, DBUser &dbu) {
    char sqlui[128];
    //sprintf(sqlui, "select `user_id`,`user_type`,`user_truename`,`user_sex`,`user_height`,`user_weight`,"
    //               "`user_job`,`user_national`,`user_birthday`,`user_pic`,`user_experience` from user_info"
    //               " where `user_id`='%d'", user_id); 
    sprintf(sqlui, "select * from `user_info` where `user_id`='%d'", user_id);
    int ret = 0;
    StoreQueryResult result;
    if (getStoreData(sqlui, result)) {
        if (result.num_rows() > 0 && result[0].size() >= 11) {
            /*
            dbu.user_id = atoi(result[0][0].c_str());
            dbu.type = result[0][1].c_str();
            dbu.truename = result[0][2].c_str();
            dbu.sex = result[0][3].c_str();
            dbu.height = result[0][4].c_str();
            dbu.weight = result[0][5].c_str();
            dbu.job = result[0][6].c_str(); 
            dbu.place = result[0][7].c_str();
            dbu.headurl = result[0][8].c_str();
            dbu.experience = result[0][9].c_str();
            */
        }
        ret = 1;
    }
    return ret;
}

int DBManager::getUserState(int user_id, DBUser &dbu) {
    char sqlu[256];
    sprintf(sqlu, "select `state`,`invited` from user_state where `user_id`='%d'", user_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlu, res)) {
        if (res.num_rows() > 0 && res[0].size() >= 2) {
            dbu.state = atoi(res[0][0].c_str());
            dbu.invited = atoi(res[0][1].c_str());
        }
        else {
            //TODO insert record
        }
        ret = 1;
    } 
    return ret;
}

int DBManager::getFriends(int user_id, vector <DBFriend> &dbfriends) {
    char sqlf[256];
    sprintf(sqlf,"select `friend_id`,`type` from `user_friends` where `user_id`='%d'", user_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlf, res)) { 
        if (res.num_rows() > 0) {
            dbfriends.resize(res.num_rows());
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() >= 2) {
                dbfriends[i].dbuser.user_id = atoi(res[i][0].c_str());
                dbfriends[i].type = res[i][1].c_str();       
            }
        }
        ret = 1;
    } 
    return ret;
}

int DBManager::getGroupInfo(int group_id, DBGroup &dbgroup) {
    char sqlgi[256]; 
    sprintf(sqlgi, "select `name`,`notice`,`headurl` from `user_group_info` where `group_id`='%d'", group_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlgi, res)) {
        if (res.num_rows() > 0 && res[0].size() >= 3) {
            dbgroup.name = res[0][0].c_str();
            dbgroup.notice = res[0][1].c_str();
            dbgroup.headurl = res[0][2].c_str();
        }    
        ret = 1;
    }
    return ret;
}

int DBManager::getGroupMembers(int group_id, DBGroup &dbgroup) {
    char sqlgm[256]; 
    sprintf(sqlgm, "select `user_id` from `user_group_members` where `group_id`='%d'", group_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlgm, res)) {
        if (res.num_rows() > 0) {
            dbgroup.members.resize(res.num_rows());
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() >= 1) {
                dbgroup.members[i].user_id = atoi(res[i][0].c_str());
            }
        }
        ret = 1;
    }
    return ret;
}

int DBManager::getUserGroups(int user_id, vector <DBGroup> &groups) {
    char sqlug[256];
    sprintf(sqlug, "select `group_id` from `user_group_members` where `user_id`='%d'", user_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlug, res)) {
        if (res.num_rows() > 0) {
            groups.resize(res.num_rows());
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() > 0) {
                groups[i].group_id = atoi(res[i][0].c_str());
            }
        }
        ret = 1;
    }
    return ret;
}

int DBManager::getTalkInfo(int talk_id, DBTalks &dbtalks) {
    char sqlti[256];
    sprintf(sqlti, "select `name`,`notice`,`headurl` from `user_talks_info` where `talk_id`='%d'", talk_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlti, res)) {
        if (res.num_rows() > 0) {
            dbtalks.members.resize(res.num_rows());
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() >= 1) {
                dbtalks.members[i].user_id = atoi(res[i][0].c_str());
            }
        }
        ret = 1;
    }
    return ret;
}

int DBManager::getTalkMembers(int talk_id, DBTalks &dbtalks) {
    char sqltm[256];
    sprintf(sqltm, "select `user_id` from `user_talks_members` where `talk_id`='%d'", talk_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqltm, res)) {
        if (res.num_rows() > 0) {
            dbtalks.members.resize(res.num_rows());
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() >= 1) {
                dbtalks.members[i].user_id = atoi(res[i][0].c_str());
            }
        }
        ret = 1;
    }
    return ret;
}

int DBManager::getUserTalks(int user_id, vector <DBTalks> &talks) {
    static char sqlut[256];
    sprintf(sqlut, "select `talk_id` from `user_talks_members` where `user_id`='%d'", user_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlut, res)) {
        if (res.num_rows() > 0) {
            talks.resize(res.num_rows());
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() > 0) {
                talks[i].talk_id = atoi(res[i][0].c_str());
            }
        }
        ret = 1;
    }
    return ret;
}

#ifdef TEST_
int main(int argc, char **argv) {
    DBManager dbm;
    dbm.init("localhost", 3306, argv[1], argv[2], "arebank2012!@");
    if (!dbm.connectMysql()) {
        cout<<"connect failed"<<endl;
        return 1;
    }
    DBUser dbu;
    int ret = 0;
    if (dbm.getUser(argv[3], dbu)) {
        cout<<dbu.user_id<<" "<<dbu.user_name<<" "<<dbu.user_pwd<<" "<<dbu.user_pwd_key;
        cout<<endl;
        string pwdstr = argv[4] + dbu.user_pwd_key;
        string pwdkey = md5(pwdstr);
        if (pwdkey == dbu.user_pwd) {
            cout<<"login success"<<endl;
        }
    }
    if (dbu.user_id != -1) {
        ret = dbm.getUserInfo(dbu.user_id, dbu);
        if (ret) {

        }
        ret = dbm.getUserState(dbu.user_id, dbu);
        if (ret) {
            cout<<dbu.user_id<<" "<<dbu.state<<" "<<dbu.invited<<endl;	
	}
        vector <DBFriend> dbfriends;
        ret = dbm.getFriends(dbu.user_id, dbfriends);
        if (ret) {
            for (size_t i = 0; i < dbfriends.size(); i++) {
	        cout<<dbfriends[i].dbuser.user_id<<" "<<dbfriends[i].type<<endl;
            }
        }
        vector <DBGroup> usergroups;
        ret = dbm.getUserGroups(dbu.user_id, usergroups);
        if (ret) {
            cout<<"user:"<<dbu.user_id<<" groups:"<<endl;
            for (size_t i = 0; i < usergroups.size(); i++) {
                cout<<usergroups[i].group_id<<" "<<endl;
                if (dbm.getGroupInfo(usergroups[i].group_id, usergroups[i])) {
                    cout<<usergroups[i].name<<" "<<usergroups[i].notice<<" "<<usergroups[i].headurl<<endl;
                } 
                if (usergroups[i].group_id > 0) {
               	    if (dbm.getGroupMembers(usergroups[i].group_id, usergroups[i])) {
			cout<<"members:"<<endl;
                        for (size_t j = 0; j < usergroups[i].members.size(); j++) {
			    cout<<usergroups[i].members[j].user_id<<" ";
 			}
			cout<<endl;
		    }
 		}
            }
        }
    }
    else {
        cout<<"get User failed"<<endl;
        return 1;
    }
    return 0;
}

#endif

