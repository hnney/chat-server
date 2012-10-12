#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <libconfig.h++>
using namespace libconfig;

#include <string>

using namespace std;

class AppConfig {

public:

    AppConfig(const char* filename); 
    ~AppConfig();

    int load_config(const char* filename = NULL);

    string& get_ls_ip() { return logic_server_ip_; }
    int get_ls_port() { return logic_server_port_;}

    string& ds_file_savepath() { return ds_file_save_path_; }

    string& get_db_host() { return dbhost_;}
    string& get_db_database() { return dbdatabase_; } 
    string& get_db_user() { return dbuser_; }
    string& get_db_pwd() { return dbpwd_; }
    int     get_db_port() { return dbport_; }

private:
    string logic_log_file_;
    string logic_server_ip_;
    int logic_server_port_;

    string ds_log_file_;
    string ds_file_save_path_;

    string dbhost_;
    string dbdatabase_;
    string dbuser_;
    string dbpwd_;
    int    dbport_;

    Config cfg_;
    string cfg_file_;
};

#endif//__CONFIG_H__

