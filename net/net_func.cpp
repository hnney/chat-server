#include "net_func.h"
#include "../common/utils.h"
#include "../common/def.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

int hl_init_socket(const char *address, int port) {
    int sockfd = -1; 
    struct sockaddr_in addr;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        return -1; 
    }   

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, address, &addr.sin_addr);
    addr.sin_port = htons(port);

    int optval = 1;
    //make address reusable
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    //turn off Nagle's algorithm
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(sockfd);
        return -2; 
    }   
    if (listen(sockfd, 1024) == -1) {
        close(sockfd);
        return -3; 
    }   
    return sockfd;
}

int set_nonblock_fd(int fd) {
    int flag = fcntl(fd, F_GETFL, 0); 
    if ( fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0) {
        return -1; 
    }   
    return 0;
}

int open_listener(const char *addr, int port) {
    int fd = hl_init_socket(addr, port);
    if (fd < 0) {
        return -1; 
    }
    if (set_nonblock_fd(fd) < 0) {
        close(fd);
        return -2;
    }
    return fd;
}

int myconnect(const char* ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) {
        cerr<<"socket failed!\n";
        return -1; 
    }   
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr<<"connect ip:"<<ip<<" port:"<<port<<" failed \n";
        close(sockfd);
        return -2; 
    }   
    return sockfd;
}

conn_t *conn_to_server(const char* ip, int port) {
    struct conn_t *conn = NULL;
    int sockfd = myconnect(ip, port);
    if (sockfd < 0) {
        return conn;
    }
    if (set_nonblock_fd(sockfd) < 0) {
        cerr<<"set fd:"<<sockfd<<" non block failed"<<endl;
        close(sockfd);
        return conn;
    }   
    conn = create_conn(sockfd, 256*1024, 256*1024);
    if (conn == NULL) { 
        cerr<<"create conn failed"<<endl;
        close(sockfd);
    }   
    return conn;
}   

int reconn_to_server(conn_t *conn, const char* ip, int port) {
    cout<<"reconnect, ip:"<<ip<<" port:"<<port<<endl; 
    if (conn->fd > 0) {
        close(conn->fd);
        conn->fd = -1;
    }   
    conn->invalid = 0;
    conn->fd = myconnect(ip, port);
    if (conn->fd <= 0) {
        conn->invalid = 1;
        return 0;
    }
    if (set_nonblock_fd(conn->fd) < 0) {
        cerr<<"reconnect, set nonblock failed"<<endl;
        conn->invalid = 1;
    }   
    return 0;
}   

int check_connected(conn_t *conn, const char* ip, int port) {
    if (conn->invalid) {
        for (int i = 0; i < 3; i++) {
            reconn_to_server(conn, ip, port);
            if (conn->invalid == 0) {
                break;
            }   
            sleep(1);
        }   
    }   
    return 0;
}  

conn_t* create_conn(int fd, int readbuf_size, int writebuf_size) {
    struct conn_t *conn = (struct conn_t *)malloc(sizeof(conn_t));
    if (conn) {
        conn->fd = fd;
        conn->readbuf_size = readbuf_size;
        conn->writebuf_size = writebuf_size;
        conn->read_pos = 0;
        conn->write_pos = 0;
        conn->invalid = 0;
        conn->ptr = NULL;
        conn->invalid_time = hl_timestamp() + CONN_INVALID_TIME;
        conn->readbuf = (char *)malloc(sizeof(char)*readbuf_size);
        conn->writebuf = (char *)malloc(sizeof(char)*writebuf_size);
        if (conn->readbuf == NULL || conn->writebuf == NULL) {
            destroy_conn(conn);
            conn = NULL;
        }
    }
    return conn;
}

void destroy_conn(conn_t *conn) {
    if (conn) {
        if (conn->readbuf) {
            free(conn->readbuf);
        }
        if (conn->writebuf) {
            free(conn->writebuf);
        }
        if (conn->fd > 0) {
            close(conn->fd);
        }
        free(conn);
    }
}

int fill_buffer(struct conn_t *conn)
{
    int readsz = 0;
    while (1) {
        int bufsize = conn->readbuf_size - conn->read_pos;
        if (bufsize <= 0) {
            cerr<<"read buffer not enough, fd:"<<conn->fd<<endl;
            return -1; 
        }   
        int n = read(conn->fd, conn->readbuf + conn->read_pos, bufsize);
        if (n < 0 && errno != EAGAIN) {
            cerr<<"read from socket error, fd:"<<conn->fd<<" errno:"<<errno<<endl;
            conn->invalid = 1; 
            return -1; 
        }   
        if (n <= 0) {
            if (n == 0) {
                conn->invalid = 1;
            }   
            break;
        }   
        readsz += n;
        conn->read_pos += n;
    }   
    return readsz;
}
int send_buffer(struct conn_t *conn)
{
    int writesz = 0;
    while (1) {
        int n = write(conn->fd, conn->writebuf + writesz, conn->write_pos - writesz);
        if (n < 0 && errno != EAGAIN) {
            cerr<<"write to socket error, fd:"<<conn->fd<<" errno:"<<errno<<endl;
            conn->invalid = 1; 
            return -1;
        }
        if (n <= 0) {
            break;
        }
        writesz += n;
    }
    if (writesz) {
        memmove(conn->writebuf, conn->writebuf + writesz, conn->write_pos - writesz);
        conn->write_pos -= writesz;
    }
    return writesz;
}

int send_data(struct conn_t *conn, const void *buf, int len)
{
    if (conn->writebuf_size - conn->write_pos < len) {
        cerr<<"send buffer not enough, fd:"<<conn->fd<<" wpos:"<<conn->write_pos<<" len:"<<len<<" mark:"<<conn->mark<<endl;
        return -1;
    }
    memcpy(conn->writebuf + conn->write_pos, buf, len);
    conn->write_pos += len;
    if (send_buffer(conn) < 0) {
        return -1;
    }
    return len;
}

int read_data(struct conn_t *conn, void *buf, int len)
{
    if (conn->read_pos < len && fill_buffer(conn) < 0) {
        return -1;
    }
    int size = min(len, conn->read_pos);
    memcpy(buf, conn->readbuf, size);
    memmove(conn->readbuf, conn->readbuf + size, conn->read_pos - size);
    conn->read_pos -= size;
    return size;
}

int send_to_client(msg_t *msg, conn_t* conn) {
    if (conn == NULL || msg == NULL) {
        cerr<<"msg or conn is null"<<endl;
        return -1;
    }
    int bufsize = conn->writebuf_size - conn->write_pos;
    int datalen = msg->serialize_size();
    if (bufsize < datalen + 6) {
        cerr<<"send buffer not enough, buf:"<<bufsize<<" datalen:"<<datalen<<endl; 
        return -1; 
    }   
    char *buf = conn->writebuf + conn->write_pos;
    sprintf(buf, "%05X@", datalen);
    msg->serialize(buf+6);
    conn->write_pos += datalen + 6;
    send_buffer(conn);
    return 0;
}

int send_msg(conn_t* conn, msg_t *msg) {
    int bufsize = conn->writebuf_size - conn->write_pos;
    int datalen = msg->serialize_size();
    if (bufsize < datalen + 6) {
        cerr<<"send buffer not enough, buf:"<<bufsize<<" datalen:"<<datalen<<endl; 
        return -1; 
    }   
    char *buf = conn->writebuf + conn->write_pos;
    sprintf(buf, "%05X@", datalen);
    msg->serialize(buf+6);
    conn->write_pos += datalen + 6;
    send_buffer(conn);
    return 0;
}

