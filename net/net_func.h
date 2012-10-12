#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

#include "../common/common.h"
#include "../common/msg.h"

int hl_init_socket(const char *address, int port);
int set_nonblock_fd(int fd);
int open_listener(const char *addr, int port); 

int myconnect(const char* ip, int port);
conn_t *conn_to_server(const char* ip, int port);
int reconn_to_server(conn_t *conn, const char* ip, int port);
int check_connected(conn_t *conn, const char* ip, int port);

conn_t* create_conn(int fd, int readbuf_size, int writebuf_size);
void destroy_conn(conn_t *conn);

int fill_buffer(struct conn_t *conn);
int send_buffer(struct conn_t *conn);
int send_data(struct conn_t *conn, const void *buf, int len);
int read_data(struct conn_t *conn, void *buf, int len);
int send_to_client(msg_t *msg, conn_t* conn);

#endif //__NET_UTILS_H__

