#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <libconfig.h++>
using namespace libconfig;

#include <string>
#include <vector>

using namespace std;

class AppConfig {

public:

    AppConfig(const char* filename); 
    ~AppConfig();

    int load_config(const char* filename = NULL);

    string& get_ls_client_bind_ip() { return logic_server_client_bind_ip_; }
    int get_ls_client_bind_port() { return logic_server_client_bind_port_;}

    string& get_ls_ds_bind_ip() { return logic_server_ds_bind_ip_;}
    int get_ls_ds_bind_port() { return logic_server_ds_bind_port_;}

    string& ds_file_savepath() { return ds_file_save_path_; }

    string& get_db_host() { return dbhost_;}
    string& get_db_database() { return dbdatabase_; } 
    string& get_db_user() { return dbuser_; }
    string& get_db_pwd() { return dbpwd_; }
    int     get_db_port() { return dbport_; }

    string& get_ds_ip(int index) { return ds_server_ip_[index]; }
    int     get_ds_port(int index) { return ds_server_port_[index];}
    int     get_ds_number() { return ds_number_; }
    int     get_ds_thread_number() { return ds_thread_number_;}

private:
    string logic_log_file_;
    string logic_server_client_bind_ip_;
    int logic_server_client_bind_port_;
    
    string logic_server_ds_bind_ip_;
    int logic_server_ds_bind_port_;

    int ds_number_;
    int ds_thread_number_;

    vector <string> ds_server_ip_;
    vector <int>    ds_server_port_;

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

