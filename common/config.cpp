#include "config.h"

AppConfig::AppConfig(const char* filename) {
    cfg_file_ = filename;
}

AppConfig::~AppConfig() {
}

int AppConfig::load_config(const char* filename) {
    const char* file = filename;
    if (file == NULL) {
        file = cfg_file_.c_str();
    }
    try {
        cfg_.readFile(file);
        
        logic_server_client_bind_ip_ = (const char *)cfg_.lookup("app.logic_server.client_bind_ip"); 
        logic_server_client_ip_ = (const char *)cfg_.lookup("app.logic_server.client_ip"); 
        logic_server_client_bind_port_ = cfg_.lookup("app.logic_server.client_bind_port"); 
        logic_server_client_port_ = cfg_.lookup("app.logic_server.client_port"); 

        logic_server_ds_bind_ip_ = (const char *)cfg_.lookup("app.logic_server.dataserver_ip");
        logic_server_ds_bind_port_ = cfg_.lookup("app.logic_server.dataserver_port");

        ds_number_ = cfg_.lookup("app.data_server.process_number");
        ds_thread_number_ = cfg_.lookup("app.data_server.thread_number_per_ds");

        ds_file_save_path_ = (const char *)cfg_.lookup("app.data_server.file_save_path");
        dbhost_ = (const char *)cfg_.lookup("app.data_server.db_host");
        dbdatabase_ = (const char *)cfg_.lookup("app.data_server.db_database");
        dbuser_ = (const char *)cfg_.lookup("app.data_server.db_user");
        dbpwd_ = (const char *)cfg_.lookup("app.data_server.db_pwd");
        dbport_ = cfg_.lookup("app.data_server.db_port");

    } catch (ParseException ex) {    
        fprintf(stderr, "Parsing config file %s failed at line %d\n", file,  ex.getLine());
        return -1;
    }    
    catch (FileIOException ex) {
        fprintf(stderr, "Read config file %s failed. IOExcetpion.\n", file); 
        return -1;
    }    
    catch (SettingNotFoundException ex) {
        fprintf(stderr, "Read config file %s failed. Setting \"%s\" not found.\n", file, ex.getPath());
        return -1;
    }    
    return 0;
}
