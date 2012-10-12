#include "../net/net_func.h"
#include "../common/dbmanager.h"
#include "../event/event_queue.h"
#include "data_proc.h"

#include "../common/config.h"


#include <unistd.h>
#include <string>
#include <signal.h>
#include <pthread.h>

#include <iostream>

using namespace std;

static string serv_ip = "";
static int port = 0;

AppConfig config("server.cfg");

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
            cerr<<"proc_data: allocate memory faield!\n";
            break;
        }   
        read_data(conn, buf, ulen+6);
        buf[ulen+6-1] = '\0';

        msg_t *msg = new msg_t();
        msg->unserialize(buf+6);
        msg_event.push_back(msg);

        if (buf != sbuf) {
            free(buf);
            buf = NULL;
        }   
    }
    push_proc_event(msg_event);
    return 0;
}

int send_event(conn_t *conn) {
    msg_t *msg = pop_proc_event();
    while (msg != NULL) {        
        if (send_to_client(msg, conn) < 0) {
            push_proc_event(msg); 
            break;
        }
        delete msg;
        msg = pop_proc_event();
    }
    return 0;
}

void *event_thread(void *arg) {
    conn_t *conn = conn_to_server(config.get_ls_ip().c_str(), config.get_ls_port());
    if (conn == NULL) {
        return NULL;
    }

    for(;;) {
        //read
        check_connected(conn, config.get_ls_ip().c_str(), config.get_ls_port());
        if (conn->invalid == 0) {
            fill_buffer(conn);
            read_event(conn);
        }
        //write
        check_connected(conn, config.get_ls_ip().c_str(), config.get_ls_port());
        if (conn->invalid == 0) {
            send_event(conn);
        }
    }

    destroy_conn(conn);
    pthread_exit(NULL);
    return NULL;
}

void *proc_thread(void *arg) {
    if (arg == NULL) {
        cerr<<" proc_thread args is null\n";
        return NULL;
    }
    for(;;) {
        msg_t *msg = pop_proc_event(); 
        if (proc_cmd(msg, arg) == 0) {
            push_send_event(msg);
        }
        else {
            push_proc_event(msg);
        }
    }
    pthread_exit(NULL);
    return NULL;
}

static void sig_handler(const int sig) {
    cout<<"SIGINT handled. \n";
    exit(0);
}

int main(int argc, char **argv) {
    
    if (argc < 4) {
        cerr<<"usage: \n";
        cerr<<"      "<<argv[0]<<" serv_ip port thread_count\n";
        exit(1);
    }

    if (config.load_config() < 0 ) {
        cerr<<"read server.cfg failed"<<endl;
        exit(1);
    }

    serv_ip = argv[1]; 
    port = atoi(argv[2]);
    
    int thread_cnt = atoi(argv[3]);
    if (thread_cnt < 2) {
        thread_cnt = 2;
    }

    if (sigignore(SIGPIPE) == -1) {
        cerr<<"failed to ignore SIGPIP, sigaction\n"; 
        exit(1);
    }
    //handle SIGINT
    signal(SIGINT, sig_handler);


    //nio thread
    pthread_t nio_thd;
    if (pthread_create(&nio_thd, NULL, event_thread, NULL) != 0) {
        cerr<<"create net io thread failed\n";
        exit(1);
    }

    pthread_t *work_thd = (pthread_t *)malloc(sizeof(pthread_t) * thread_cnt);
    if (work_thd == NULL) {
        cerr<<"mallock pthread_t* failed\n";
        exit(1);
    }

    DBManager * dbm = new DBManager[thread_cnt];
    for (int i = 0; i < thread_cnt; i++) {
        dbm[i].init(config.get_db_host().c_str(), config.get_db_port(), config.get_db_database().c_str(),
                config.get_db_user().c_str(), config.get_db_pwd().c_str());
        dbm[i].connectMysql();
        if (pthread_create(&work_thd[i], NULL, proc_thread, (void*)&dbm[i]) != 0) {
            cerr<<"create work thd["<<i<<"] failed\n";
        }
    }

    //暂时让他不结束
    sleep(365*24*3600);

    free(work_thd);
    delete dbm;
   
    return 0;
}


