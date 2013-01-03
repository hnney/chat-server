#include "../net/net_func.h"
#include "../common/dbmanager.h"
#include "../event/event_queue.h"
#include "data_proc.h"

#include "../common/config.h"


#include <unistd.h>
#include <string>
#include <signal.h>
#include <pthread.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/logger.h>

#include <iostream>

using namespace std;

static volatile bool running = false;

AppConfig config_("server.cfg");
log4cxx::LoggerPtr logger_;

int read_event(conn_t *conn) {
    char sbuf[8192];
    vector <msg_t*> msg_event;
    for (;;) {
        if (conn->read_pos <= 6) {
            break;
        }   
        char *buf = sbuf;
        unsigned int ulen = 0;
        sscanf(conn->readbuf, "%05X", &ulen);
        if ((unsigned int)conn->read_pos < (6 + ulen)) {
            break;
        }   
        if (ulen + 6 > sizeof(sbuf)) {
            buf = (char *)malloc(sizeof(char)*(ulen+6));
        }   
        if (buf == NULL) {
            LOG4CXX_ERROR(logger_, "dataserver proc_data: allocate memory faield");
            break;
        }   
        read_data(conn, buf, ulen+6);
        buf[ulen+6-1] = '\0';

        LOG4CXX_DEBUG(logger_, "ulen:"<<ulen<<" buf:["<<buf<<"]");

        msg_t *msg = new msg_t();
        msg->unserialize(buf+6);
        msg_event.push_back(msg);

        if (buf != sbuf) {
            free(buf);
            buf = NULL;
        }   
    }
    int cnt = msg_event.size();
    if (cnt > 0) {
        push_proc_event(msg_event);
    }
    return cnt;
}

int send_event(conn_t *conn) {
    int cnt = 0;
    msg_t *msg = pop_send_event();
    while (msg != NULL) {        
        if (send_to_client(msg, conn) < 0) {
            push_send_event(msg); 
            break;
        }
        delete msg;
        msg = pop_send_event();
        cnt ++;
    }
    return cnt;
}

void *event_thread(void *arg) {
    conn_t *conn = conn_to_server(config_.get_ls_ds_bind_ip().c_str(), config_.get_ls_ds_bind_port());
    if (conn == NULL) {
        running = false;
        return NULL;
    }

    int rcnt = 0;
    int wcnt = 0;
    cout<<"event_thread"<<endl;

    time_t last_keepalive = 0;    

    while(running) {
        //read
        check_connected(conn, config_.get_ls_ds_bind_ip().c_str(), config_.get_ls_ds_bind_port());
        if (conn->invalid == 0) {
            fill_buffer(conn);
            rcnt = read_event(conn);
        }
        //write
        check_connected(conn, config_.get_ls_ds_bind_ip().c_str(), config_.get_ls_ds_bind_port());
        if (conn->invalid == 0) {
            wcnt = send_event(conn);
        }
        //TODO
        //如果没有任何事情可以做的话，就休息一阵
        if (rcnt == 0 && wcnt == 0) {
            usleep(300);
        }
        time_t now = hl_timestamp();
        if (now - last_keepalive > 10*1000*1000) {
            last_keepalive = now;
            cout<<"t:"<<(now-last_keepalive)<<endl;
            //ka
            send_keepalive();
        }
    }
    destroy_conn(conn);
    return NULL;
}

void *proc_thread(void *arg) {
    //log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("dataserver");
    if (arg == NULL) {
        //LOG4CXX_ERROR(logger_, "dataserver create proc_thread failed, arg is null");
        return NULL;
    }
    while(running) {
        //LOG4CXX_DEBUG(logger, "dataserver proc_thread while ... start");
        msg_t *msg = try_pop_proc_event(); 
        if (msg != NULL) {
            if (proc_cmd(msg, arg) == 0) {
                push_send_event(msg);
            }
        }
        else {
            usleep(300);
            sleep(1);
        }
        //LOG4CXX_DEBUG(logger, "dataserver proc_thread while ... ");
    }
    pthread_exit(NULL);
    return NULL;
}

static void sig_handler(const int sig) {
    cout<<"SIGINT handled. \n";
    running = false;
    exit(0);
}

int main(int argc, char **argv) {

    if (config_.load_config() < 0 ) {
        cerr<<"read server.cfg failed"<<endl;
        return 1;
    }

    if (argc < 2) {
        cerr<<"usage: ";
        cerr<<argv[0]<<" id_num"<<endl; 
        return 1;
    }

    int dsid = atoi(argv[1]);
    if (dsid < 0 || dsid > config_.get_ds_number()) {
        cerr<<"dsid is invalid"<<endl;
        return 1;
    }
    
    //logger
    string logcfg = "log.conf";
    log4cxx::PropertyConfigurator::configureAndWatch(logcfg);
    logger_ = log4cxx::Logger::getLogger("dataserver");
    if (logger_ == NULL) {
        cerr<<"getLogger dataserver failed"<<endl;
        return 1;
    }


    int thread_cnt = config_.get_ds_thread_number(); 
    /*
    if (thread_cnt < 2) {
        thread_cnt = 2;
    }
    */

    if (sigignore(SIGPIPE) == -1) {
        LOG4CXX_ERROR(logger_, "dataserver sigignore SIGPIPE failed, exit");
        return 1;
    }
    //handle SIGINT
    signal(SIGINT, sig_handler);

    running = true;

    pthread_t *work_thd = (pthread_t *)malloc(sizeof(pthread_t) * thread_cnt);
    if (work_thd == NULL) {
        LOG4CXX_ERROR(logger_, "dataserver malloc "<<thread_cnt<<" pthread_t memory failed, exit");
        return 1;
    }

    DBManager * dbm = new DBManager[thread_cnt];

    for (int i = 0; i < thread_cnt; i++) {
        dbm[i].init(config_.get_db_host().c_str(), config_.get_db_port(), config_.get_db_database().c_str(),
                config_.get_db_user().c_str(), config_.get_db_pwd().c_str());
        if (!dbm[i].connectMysql()) {
            LOG4CXX_ERROR(logger_, "dataserver connect mysql failed");
            running = false;
            break;
        }
        pthread_attr_t  attr;
        pthread_attr_init(&attr);
        if (pthread_create(&work_thd[i], &attr, proc_thread, (void*)&dbm[i]) != 0) {
            LOG4CXX_ERROR(logger_, "dataserver create proc_thread ["<<i<<"] failed");
            running = false;
            break;
        }
    }
    
    LOG4CXX_INFO(logger_, "dataserver ["<<dsid<<"] start, thnum:"<<thread_cnt);

    event_thread(NULL);

    LOG4CXX_INFO(logger_, "dataserver wait all thread exit");

    //wait for work thread exit
    for (int i = 0; i < thread_cnt; i++) {
        pthread_join(work_thd[i], NULL); 
    }
    free(work_thd);
    //destroy dbm
    delete []dbm;
    
    LOG4CXX_INFO(logger_, "dataserver end");
    
    return 0;
}


