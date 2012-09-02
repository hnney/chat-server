#include "../event/net_event.h"
#include <iostream>
#include <map>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define READ_BUFSIZE 1024*10
#define WRITE_BUFSIZE 1024*10

using namespace std;

static int client_listen_fd_ = -1;
static int ds_listen_fd_ = -1;
static event_t *handler = NULL;

//static map <int, int> uid2fd_map;
static map <int, user_t *> idu_map;
static map <int, struct conn_t *> fdc_map;
static vector <struct conn_t *> dbserver_conns;

heap <conninfo> conn_timeout;

static int accept_new_client(int fd) {
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    int c_fd = -1;
    while ((c_fd = accept(fd, (struct sockaddr*)&clientaddr, &addrlen)) > 0) {
        cout<<"accept new fd:"<<c_fd<<endl;
        if(set_nonblock_fd(c_fd) < 0) {
            cerr<<"set nonblock failed, fd:"<<c_fd<<endl;
            close(c_fd);
            continue;
        }
        conn_t *conn = create_conn(c_fd, READ_BUFSIZE, WRITE_BUFSIZE);
        if (conn == NULL) {
            cerr<<"malloc conn failed\n";
            clsoe(c_fd);
            continue;     
        }
        if (event_add(handler,c_fd, EV_READ|EV_WRITE|EV_ET) < 0) {
            cerr<<"event_add faield, fd"<<c_fd<<endl;
            close(c_fd);
            conn->fd = 0;
            destroy_conn(conn);
            continue;
        }
        conn_timeout.push(conninfo(conn));
        fdc_map[c_fd] = conn;
        if (fd == ds_listen_fd_) {
            dbserver_conns.push_back(conn);
            cout<<"data server client\n";
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
        sscanf(conn->readbuf, "%05X", ulen);
        if (conn->read_pos < 6 + ulen) {
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
        
        msg_t msg;
        msg.unserialize(buf+6);

        proc_cmd(&msg, conn);

        if (buf != sbuf) {
            free(buf);
            buf = NULL;
        }
    }
    return 0;
}

static int net_handle(int fd, int op) {
    if (fd == client_listen_fd_) {
        accept_new_client(fd);
    }
    else {
        map <int, conn_t *>::ierator fditer = fdc_map.find(fd);
        if (fditer == fdc_map.end()) {
            cerr<<"net_handler not found conn in fdc_map, fd:"<<fd<<endl;
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
            send_buffer(conn);
        }
    }
}

static void *thread_main(void *arg) {
    if (arg == NULL) {
        cerr<<"thread_main param is null\n";
        return NULL;
    }
    event_t *h = (event_t *)arg;
    while (1) {
        event_dispatch(h, 1000);

        // check conn timeout
        // invalid_time < now
        while (conn_timeout.size() > 0 && conn_timeout.top().conn->invalid_time < now) {
            conn_t *conn = conn_timeout.top().conn;
            conn_timeout.pop();
            //TODO
        }
    }
    return null;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        cout<<"usage: "<<argv[0]<<" ip port"<<endl;    
        return 1;
    }
    handler = event_init(net_handle, 10240);
    if (!handler) {
        cerr<<"event_init failed"<<endl;
        return 1;
    }
    char *ip = argv[1];
    int port = atoi(argv[2]); 
    client_listen_fd_ = open_listener(ip, port);
    if (client_listen_fd_ < 0) {
        cerr<<"open_listener() failed"<<endl;
        goto exit_;
    }

    thread_main(handler);

exit_:
    if (handler) {
        event_destroy(handler);
        handler = NULL;
    }
    return 0;
}

