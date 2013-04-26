#include "dbmanager.h"
#include "utils.h"
//g++ -g -DTEST_ $(mysql_config --cflags) dbmanager.cpp $(mysql_config --libs) libmysqlpp.so 
//
//TODO need to Security Certificate

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
    conn.set_option(new SetCharsetNameOption("gbk"));
    if (conn.connect(database_.c_str(), host_.c_str(), user_.c_str(), pwd_.c_str(), port_)){
        Query q = conn.query("set names gbk");
        q.execute();
        return true;
    }
    else {
        cerr<<"connect to data failed\n";
        return false;   
    }
}

bool DBManager::checkConnection() {
    if (conn_.ping() == false) {
        closeMysql();
        return connectMysql();
    }
    return true;
}

int DBManager::execSql(const char *sql) {
    int ret = 0;
    try {
        if (!checkConnection()) {
            cerr<<"connect mysql faield"<<endl;
            return 0;
        }
        Query query = conn_.query(sql);
        query.execute();
        ret = 1;
    }
    catch(mysqlpp::BadQuery& er) {
        cerr<<"exec sql failed, ["<<sql<<"], err:"<<er.what()<<endl;
    }
    catch (const mysqlpp::Exception& er) {
        cerr<<"exec sql failed, ["<<sql<<"], err:"<<er.what()<<endl;
    } 
    return ret;
}

int DBManager::execSql(const string &sql) {
    int ret = 0;
    try {
        if (!checkConnection()) {
            cerr<<"connect mysql faield"<<endl;
            return 0;
        }
        Query query = conn_.query(sql);
        query.execute();
        ret = 1;
    }
    catch(mysqlpp::BadQuery& er) {
        cerr<<"exec sql failed, ["<<"], err:"<<er.what()<<endl;
    }
    catch (const mysqlpp::Exception& er) {
        cerr<<"exec sql failed, ["<<"], err:"<<er.what()<<endl;
    } 
    return ret;
}

int DBManager::getStoreData(const char *sql, StoreQueryResult &result) {
    int ret = 0;
    try {
        if (!checkConnection()) {
            cerr<<"connect mysql faield"<<endl;
            return 0;
        }
        Query query = conn_.query(sql);
        //query.parse();
        result = query.store();
        if (result) {
            ret = 1;
        }
        StoreQueryResult res;
        for (int i = 1; query.more_results(); ++i) {
            res = query.store_next();
        }
    }
    catch (const mysqlpp::BadQuery& er) {
        cerr<<"getstore failed:["<<sql<<"], err:"<<er.what()<<endl;
    }
    catch (const mysqlpp::BadConversion& er) {
        cerr<<"getstare failed:["<<sql<<"], err:"<<er.what()<<endl;
    }
    catch (const mysqlpp::Exception& er) {
         cerr<<"getstare failed:["<<sql<<"], err:"<<er.what()<<endl;
    }
    catch(...) {
        cerr<<"getStareData faield, sql:"<<sql<<endl;
    }
    return ret;
}

int DBManager::getStoreData(string &sql, StoreQueryResult &result) {
    int ret = 0;
    try {
        if (!checkConnection()) {
            cerr<<"connect mysql faield"<<endl;
            return 0;
        }
        Query query = conn_.query(sql);
        //query.parse();
        result = query.store();
        if (result) {
            ret = 1;
        }
        StoreQueryResult res;
        for (int i = 1; query.more_results(); ++i) {
            res = query.store_next();
        }
    }
    catch (const mysqlpp::BadQuery& er) {
        cerr<<"getstore failed:["<<sql<<"], err:"<<er.what()<<endl;
    }
    catch (const mysqlpp::BadConversion& er) {
        cerr<<"getstare failed:["<<sql<<"], err:"<<er.what()<<endl;
    }
    catch (const mysqlpp::Exception& er) {
         cerr<<"getstare failed:["<<sql<<"], err:"<<er.what()<<endl;
    }
    catch(...) {
        cerr<<"getStareData faield, sql:"<<sql<<endl;
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

int DBManager::getUser(const string &name, DBUser &dbu) {
    //char sql[128];
    //sprintf(sql, "call user_login('%s')", name);
    string sql = "call user_login('" + name + "')";
    StoreQueryResult result;
    int ret = 0;
    if (getStoreData(sql, result)) {
        cout<<"rows:"<<result.num_rows()<<endl;
        if (result.num_rows() > 0 && result[0].size() >= 4) {
            dbu.user_id = atoi(result[0][0].c_str());
            dbu.user_name = string(result[0][1].c_str(), result[0][1].size());
            dbu.user_pwd = string(result[0][2].c_str(), result[0][2].size());
            dbu.user_pwd_key = string(result[0][3].c_str(), result[0][3].size());
            ret = 1;
        }
    }
    return ret;
}

int DBManager::getUserInfo(int user_id, DBUser &dbu) {
    char sqlui[512];
    sprintf(sqlui, "select u.`user_id`,user_info.`user_type`,user_info.`user_truename`,user_info.`user_sex`,"
                   "user_info.`user_height`,user_info.`user_weight`,user_info.`user_job`,user_info.`user_national`,"
                   "user_info.`user_birthday`,user_info.`user_pic`,user_info.`user_experience`,user_info.`user_intro`,"
                   "u.user_name from are_sys_user "
                   "u left join user_info on u.user_id=user_info.user_id "
                   " where u.`user_id`='%d'", user_id);
    //sprintf(sqlui, "select `user_id`,`user_type`,`user_truename`,`user_sex`,"
     //              "`user_height`,`user_weight`,`user_job`,`user_national`,"
      //             "`user_birthday`,`user_pic`,`user_experience` from user_info "
       //            " where `user_id`='%d'", user_id); 
    //sprintf(sqlui, "select * from `user_info` where `user_id`='%d'", user_id);
    int ret = 0;
    StoreQueryResult result;
    if (getStoreData(sqlui, result)) {
        if (result.num_rows() > 0 && result[0].size() >= 13) {
            //dbu.user_id = atoi(result[0][0].c_str());
            dbu.type = string(result[0][1].c_str(), result[0][1].size());
            dbu.truename = string(result[0][2].c_str(), result[0][2].size());
            dbu.sex = string(result[0][3].c_str(), result[0][3].size());
            dbu.height = string(result[0][4].c_str(), result[0][4].size());
            dbu.weight = string(result[0][5].c_str(), result[0][5].size());
            dbu.job = string(result[0][6].c_str(), result[0][6].size()); 
            dbu.place = string(result[0][7].c_str(), result[0][7].size());
            dbu.birthday = string(result[0][8].c_str(), result[0][8].size());
            dbu.headurl = string(result[0][9].c_str(), result[0][9].size());
            dbu.experience = string(result[0][10].c_str(), result[0][10].size());
            dbu.user_desc = string(result[0][11].c_str(), result[0][11].size());
            dbu.user_name = string(result[0][12].c_str(), result[0][12].size());
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
            sprintf(sqlu, "insert into `user_state`(`user_id`, `state`, `invited`) values('%d','0','0')",user_id);
            execSql(sqlu);
        }
        ret = 1;
    } 
    return ret;
}

int DBManager::setUserState(int user_id, int state) { 
    char sql[128];
    sprintf(sql, "update `user_state` set `state`='%d' where `user_id`='%d'", state, user_id);
    execSql(sql);
    return 1; 
}

int DBManager::setUserInvited(int user_id, int invited) {
    char sql[128];
    sprintf(sql, "update `user_state` set `invited`='%d' where `user_id`='%d'", invited, user_id);
    return execSql(sql);
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
                dbfriends[i].type = string(res[i][1].c_str(), res[i][1].size());       
            }
        }
        ret = 1;
    } 
    return ret;
}

int DBManager::addFriend(int user_id, int friend_id, string &type) {
    char sql[128];
    sprintf(sql, "insert into `user_friends`(`user_id`, `friend_id`,`type`) values('%d', '%d', '%s')", user_id, friend_id, type.c_str());
    return execSql(sql);
}

int DBManager::modifyFriend(int user_id, int friend_id, string &newtype) {
    char sql[256];
    sprintf(sql, "update `user_friends` set `type`='%s' where `user_id`='%d' and `friend_id`='%d'", newtype.c_str(), user_id, friend_id);
    execSql(sql);
    return 1;
}

int DBManager::delFriend(int user_id, int friend_id) {
    char sql[128];
    sprintf(sql, "delete from `user_friends` where `user_id`='%d' and `friend_id`='%d'", user_id, friend_id);
    execSql(sql);
    return 1;
}

int DBManager::createGroup(int user_id, string group_name, string &notice, string &headurl, int time) {
    stringstream idstr;
    idstr<<user_id;
    stringstream timestr;
    timestr<<time;
    string sql = "insert into `user_group_info`(`name`,`notice`,`headurl`,`user_id`,`create_time`) values('" +
           group_name + "','" + notice + "','" + headurl + "','" + idstr.str() + "','" + timestr.str() + "')";
    execSql(sql);
    return 1;
}
int DBManager::getGroupInfo(int user_id, int time, DBGroup &dbgroup) {
    char sql[256];
    sprintf(sql, "select `group_id`,`name`,`notice`,`headurl` from `user_group_info` where `user_id`='%d' and `create_time`='%d'", user_id, time);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sql, res)) {
        if (res.num_rows() > 0 && res[0].size() >= 4) {
            dbgroup.group_id = atoi(res[0][0].c_str());
            dbgroup.name = string(res[0][1].c_str(), res[0][1].size());
            dbgroup.notice = string(res[0][2].c_str(), res[0][2].size());
            dbgroup.notice = string(res[0][3].c_str(), res[0][3].size()); 
        }
        ret = 1;
    }
    return ret;
}

int DBManager::getGroupInfo(int group_id, DBGroup &dbgroup) {
    char sqlgi[256]; 
    sprintf(sqlgi, "select `name`,`notice`,`headurl`,`user_id` from `user_group_info` where `group_id`='%d'", group_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlgi, res)) {
        if (res.num_rows() > 0 && res[0].size() >= 4) {
            dbgroup.name = string(res[0][0].c_str(), res[0][0].size());
            dbgroup.notice = string(res[0][1].c_str(), res[0][1].size());
            dbgroup.headurl = string(res[0][2].c_str(), res[0][2].size());
            dbgroup.admin_id = atoi(res[0][3].c_str());
        }    
        ret = 1;
    }
    return ret;
}

int DBManager::modifyGroupName(int group_id, int user_id, string &name) {
    stringstream group_idstr;
    group_idstr<<group_id;
    stringstream user_idstr;
    user_idstr<<user_id; 
    string sql = "update `user_group_info` set `name`='" + name + "' where `group_id`='" + group_idstr.str() + "' and `user_id`='" + user_idstr.str() + "'";
    return execSql(sql);
}

int DBManager::getGroupMembers(int group_id, DBGroup &dbgroup) {
    char sqlgm[256]; 
    sprintf(sqlgm, "select `user_id`,`type` from `user_group_members` where `group_id`='%d'", group_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlgm, res)) {
        if (res.num_rows() > 0) {
            dbgroup.members.resize(res.num_rows());
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() >= 2) {
                dbgroup.members[i].user_id = atoi(res[i][0].c_str());
                dbgroup.members[i].subtype = atoi(res[i][1].c_str());
            }
        }
        ret = 1;
    }
    return ret;
}

int DBManager::addGroupMember(int group_id, int member_id, int member_type) {
    char sql[256];
    sprintf(sql, "insert into `user_group_members`(`group_id`, `user_id`, `type`) values('%d', '%d', '%d')", group_id, member_id, member_type);
    return execSql(sql);
}

int DBManager::getGroupMemberType(int group_id, int member_id) {
    char sql[256];
    int type = -1;
    sprintf(sql, "select `type` from `user_group_members` where `group_id`='%d' and `user_id`='%d'", group_id, member_id);
    StoreQueryResult res;
    if (getStoreData(sql, res)) {
        if (res.num_rows() > 0 && res[0].size() > 0) {
            type = atoi(res[0][0].c_str());
        }
    }
    return type;
}

int DBManager::delGroupMember(int group_id, int member_id) {
    char sql[256];
    sprintf(sql, "delete from `user_group_members` where `group_id`='%d' and `user_id`='%d'", group_id, member_id);
    return execSql(sql);
}
int DBManager::delGroupMember(int group_id) {
    char sql[256];
    sprintf(sql, "delete from `user_group_members` where `group_id`='%d'", group_id);
    execSql(sql);
    sprintf(sql, "update `user_group_info` set `invalid`='1' where `group_id`='%d'", group_id);
    return execSql(sql);
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

int DBManager::searchGroups(string match_name, vector <DBGroup> &groups) {
    string sql = "select `group_id`,`name` from `user_group_info` where `name` like \"%" + match_name + "%\" and `invalid`='0'";
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sql, res)) {
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() > 1) {
                groups.push_back(DBGroup());
                groups[i].group_id = atoi(res[i][0].c_str());
                groups[i].name = string(res[i][1].c_str(), res[i][1].size());
            }
        }
        ret = 1;
    }
    return ret; 
}

int DBManager::getTalkInfo(int talk_id, DBTalks &dbtalks) {
    char sqlti[256];
    sprintf(sqlti, "select `name`,`notice`,`headurl` from `user_talks_info` where `talks_id`='%d'", talk_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sqlti, res)) {
        if (res.num_rows() > 0 && res[0].size() >= 3) {
            dbtalks.name = string(res[0][0].c_str(), res[0][0].size());
            dbtalks.notice = string(res[0][1].c_str(), res[0][1].size());
            dbtalks.headurl = string(res[0][2].c_str(), res[0][2].size());
        }   
        ret = 1;
    }
    return ret;
}

int DBManager::getTalkMembers(int talk_id, DBTalks &dbtalks) {
    char sqltm[256];
    sprintf(sqltm, "select `user_id` from `user_talks_members` where `talks_id`='%d'", talk_id);
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
    char sqlut[256];
    sprintf(sqlut, "select `talks_id` from `user_talks_members` where `user_id`='%d'", user_id);
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

// msg to db
int DBManager::getUserMessages(int user_id, vector <string> &messages) {
    char sql[512];
    sprintf(sql, "select `messages` from `user_messages` where `user_id`='%d'", user_id);
    int ret = 0;
    StoreQueryResult res;
    if (getStoreData(sql, res)) {
        if (res.num_rows() > 0) {
            messages.resize(res.num_rows());      
        }
        for (size_t i = 0; i < res.num_rows(); i++) {
            if (res[i].size() > 0) {
                string str(res[i][0].c_str(), res[i][0].size());
                messages[i] = base64_decode(str);
            }
        } 
        ret = 1;
    }
    return ret;
}

int DBManager::setUserMessages(int user_id, const string &messages) {
    stringstream idstr;
    idstr<<user_id;
    char *strDst = new char[messages.size() * 4 + 1];  
    strDst[messages.size()*4] = '\0';
    base64_encode(messages.c_str(), messages.size(), strDst, messages.size() * 4 + 1);
    string sql = "insert into `user_messages`(`user_id`, `messages`) values('" + idstr.str() + "','" + strDst + "')"; 
    execSql(sql);
    delete []strDst;
    return 1;
}

int DBManager::deleteUserMessages(int user_id) {
    char sql[512];
    sprintf(sql, "delete from `user_messages` where `user_id`='%d'", user_id);
    execSql(sql);
    return 1;
}


int DBManager::recordTextMessage(int send_id, string send_uid, string recv_uid, const char *text, int textlen, int type/*=0*/, int group_id/*=0*/) {
    stringstream idstr;
    idstr<<send_id;
    stringstream typestr;
    typestr<<type;
    stringstream group_idstr;
    group_idstr<<group_id;
    char *strDst = new char[textlen*4 + 1];
    strDst[textlen*4] = '\0';
    base64_encode(text, textlen, strDst, textlen*4+1);
    string sql = "insert into `text_record`(`send_id`, `send_uid`, `recv_uid`, `type`, `text`, `group_id`) values('" +  
                 idstr.str() + "','" + send_uid + "','" + recv_uid + "','" + typestr.str() + "','" + string(strDst) + 
                 "','" + group_idstr.str() + "')";
    delete []strDst;
    return execSql(sql);
}

int DBManager::report(int from_user_id, int from_ip, string tuid, const char* content, int contentlen) {
    stringstream from_idstr;
    from_idstr<<from_user_id;
    stringstream ipstr;
    ipstr<<from_ip;
    string strcontent = string(content, contentlen);
    string sql = "insert into `are_hr_report`(`report_user_name`, `report_from_uid`, `report_ip`, `report_content`) values('" +
                 tuid + "','" + from_idstr.str() + "','" + ipstr.str() + "','" + strcontent + "')";
    return execSql(sql);
}


#ifdef TEST_
//dbname user pwd uid,uid_pwd
int main(int argc, char **argv) {
    DBManager dbm;
    string p = string(argv[3]) + "!@";
    dbm.init("localhost", 3306, argv[1], argv[2], p.c_str());
    if (!dbm.connectMysql()) {
        cout<<"connect failed"<<endl;
        return 1;
    }
    DBUser dbu;
    int ret = 0;
    if (dbm.getUser(argv[4], dbu)) {
        cout<<dbu.user_id<<" "<<dbu.user_name<<" "<<dbu.user_pwd<<" "<<dbu.user_pwd_key;
        cout<<endl;
        string pwdstr = argv[5] + dbu.user_pwd_key;
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
        dbm.setUserState(dbu.user_id, 1);
    }
    else {
        cout<<"get User failed"<<endl;
        return 1;
    }
    return 0;
}

#endif

