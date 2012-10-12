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
        
        logic_server_ip_ = (const char *)cfg_.lookup("app.logic_server.ip"); 
        logic_server_port_ = cfg_.lookup("app.logic_server.port"); 

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
