#include "../common/common.h"
#include "../common/heap.h"
#include "../common/msg.h"
#include "../common/utils.h"
#include "../common/config.h"
#include "../net/net_event.h"
#include "../net/net_func.h"
#include "logic_proc.h"

#include <iostream>
#include <map>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/logger.h>


#define READ_BUFSIZE 1024*10
#define WRITE_BUFSIZE 1024*10

using namespace std;


static volatile bool running_ = false;

AppConfig config_("server.cfg");
log4cxx::LoggerPtr logger_;

static int client_listen_fd_ = -1;
static int ds_listen_fd_ = -1;
static event_t *handler = NULL;

map <string, user_t *> idu_map;
map <int, struct conn_t *> fdc_map;
vector <struct conn_t *> dbserver_conns;

heap <conninfo, greater<conninfo> > conn_timeout;

static int accept_new_client(int fd) {
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    int c_fd = -1;
    while ((c_fd = accept(fd, (struct sockaddr*)&clientaddr, &addrlen)) > 0) {
        LOG4CXX_DEBUG(logger_, "logicserver accept new fd:"<<c_fd);
        if(set_nonblock_fd(c_fd) < 0) {
            LOG4CXX_ERROR(logger_, "set nonblock failed, fd:"<<c_fd);
            close(c_fd);
            continue;
        }
        conn_t *conn = create_conn(c_fd, READ_BUFSIZE, WRITE_BUFSIZE);
        if (conn == NULL) {
            LOG4CXX_ERROR(logger_, "malloc conn failed");
            close(c_fd);
            continue;     
        }
        if (event_add(handler,c_fd, EV_READ|EV_WRITE|EV_ET) < 0) {
            LOG4CXX_ERROR(logger_, "event_add failed ,fd:"<<c_fd);
            close(c_fd);
            conn->fd = 0;
            destroy_conn(conn);
            continue;
        }
        conn_timeout.push(conninfo(conn));
        fdc_map[c_fd] = conn;

        if (fd == ds_listen_fd_) {
            conn->mark = CONN_DB_SERVER;
            dbserver_conns.push_back(conn);
            LOG4CXX_DEBUG(logger_, "data server conn, fd:"<<c_fd);
        }
        else {
            conn->mark = CONN_CLIENT;
        }
    } 
    return 0;
}

static int proc_data(conn_t* conn) {
    static char sbuf[8192];
    while (1) {
        //proc all msg event
        if (conn->read_pos <= 6) {
            break;
        }
        char *buf = sbuf;
        unsigned int ulen = 0;
        sscanf(conn->readbuf, "%05X@", &ulen);
        if ((unsigned int)conn->read_pos < (6 + ulen)) {
            break;
        }
        if (ulen + 6 > sizeof(sbuf)) {
            buf = (char *)malloc(sizeof(char)*(ulen+6));
        }
        if (buf == NULL) {
            LOG4CXX_ERROR(logger_, "logicserver proc_data allocate memory failed");
            break;
        }
        read_data(conn, buf, ulen+6);
        buf[ulen+6-1] = '\0';

        LOG4CXX_DEBUG(logger_, "ulen:"<<ulen<<" logicserver buf:["<<buf<<"]");

        msg_t msg;
        msg.unserialize(buf+6);

        if (conn->mark == CONN_CLIENT) {
            //TODO
            msg.set_state(1);
        }
        proc_cmd(&msg, conn);

        if (buf != sbuf) {
            free(buf);
            buf = NULL;
        }
    }
    return 0;
}

static int net_handle(int fd, int op) {
    if (fd == client_listen_fd_ || fd == ds_listen_fd_) {
        accept_new_client(fd);
    }
    else {
        map <int, conn_t *>::iterator fditer = fdc_map.find(fd);
        if (fditer == fdc_map.end()) {
            LOG4CXX_ERROR(logger_, "logicserver net_handler not found conn in fdc_map, fd:"<<fd);
            return -1; 
        }
        conn_t *conn = fditer->second;
        int rl = 0, wl = 0;
        if (op & EV_READ) {
            rl = fill_buffer(conn); 
            //read data
            proc_data(conn);
        }
        if(op & EV_WRITE) {
            //write data
            wl = send_buffer(conn);
        }
        LOG4CXX_DEBUG(logger_, "logicserver rec:"<<rl<<" wl:"<<wl);
    }
    return 0;
}

static void *thread_main(void *arg) {
    if (arg == NULL) {
        LOG4CXX_ERROR(logger_, "thread_main arg is null");
        return NULL;
    }
    event_t *h = (event_t *)arg;
    while (running_) {
        event_dispatch(h, 3000);
        // check conn timeout
        // invalid_time < now
        long long now = hl_timestamp();
        while (conn_timeout.size() > 0 && conn_timeout.top().conn->invalid_time < now) {
            conn_t *conn = conn_timeout.top().conn;
            conn_timeout.pop();

            //TODO
            if (conn->data.ptr != NULL) {
                user_t *user = (user_t *)conn->data.ptr;
                send_user_exit(user);
                clean_user(user);
            } 
            else {
                clean_conn(conn);
            }
        }
    }
    return NULL;
}

static void sig_handler(const int sig) {
    LOG4CXX_DEBUG(logger_, "logicserver sig_handler "<<sig);
    if (sig == SA_RESTART) {
        return;
    }
    LOG4CXX_ERROR(logger_, "logicserver sig_handler "<<sig<<", exit");
    running_ = false;
}

int main(int argc, char **argv) {
    
    /*
    if (argc < 3) {
        cerr<<"usage: "<<argv[0]<<" ip port"<<endl;    
        return 1;
    }
    */

    if (config_.load_config() < 0) {
        cerr<<"logicserver read server.cfg failed"<<endl;
        return 1;
    }

    if (argc < 2) {
        cout<<"usage: ";
        cout<<argv[0]<<" id"<<endl;
        return 1;
    }

    string logcfg = "log.conf";
    log4cxx::PropertyConfigurator::configureAndWatch(logcfg);
    logger_ = log4cxx::Logger::getLogger("logicserver");
    if (logger_ == NULL) {
        cerr<<"logicserver getLogger "<<logcfg.c_str()<<" failed"<<endl;
        return 1;
    }

    if (sigignore(SIGPIPE) == -1) {
        LOG4CXX_ERROR(logger_, "logicserver sigignore SIGPIPE failed");
        return 1;
    }   
    //handle SIGINT
    signal(SIGINT, sig_handler);

    handler = event_init(net_handle, 10240);
    if (!handler) {
        LOG4CXX_ERROR(logger_, "logicserver event_init failed");
        return 1;
    }

    const char *ip = config_.get_ls_client_bind_ip().c_str();
    int port = config_.get_ls_client_bind_port(); 
    client_listen_fd_ = open_listener(ip, port);
    if (client_listen_fd_ < 0) {
        LOG4CXX_ERROR(logger_, "logicserver open_listener() failed, ip:"<<ip<<", port:"<<port);
        event_destroy(handler);
        return 1;
    }
    if (event_add(handler, client_listen_fd_, EV_READ|EV_WRITE|EV_ET) < 0) {
        LOG4CXX_ERROR(logger_, "event_add failed ,fd:"<<client_listen_fd_);
        close(client_listen_fd_);
        return 1;
    } 
    
    ip = config_.get_ls_ds_bind_ip().c_str();
    port = config_.get_ls_ds_bind_port();
    ds_listen_fd_ = open_listener(ip, port);
    if (ds_listen_fd_ < 0) {
        close(client_listen_fd_);
        LOG4CXX_ERROR(logger_, "logicserver open_listener() failed, ip:"<<ip<<", port:"<<port);
        event_destroy(handler);
        return 1;
    }
    if (event_add(handler, ds_listen_fd_, EV_READ|EV_WRITE|EV_ET) < 0) {
        LOG4CXX_ERROR(logger_, "event_add failed ,fd:"<<ds_listen_fd_);
        close(ds_listen_fd_);
        return 1;
    } 
    running_ = true;

    thread_main(handler);

    event_destroy(handler);

    return 0;
}

