#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

int hl_init_socket(const char *address, int port);

int set_nonblock_fd(int fd);

int open_listener(const char *addr, int port); 

conn_t* create_conn(int fd, int readbuf_size, int writebuf_size);

int fill_buffer(struct conn_t *conn)
{
    int readsz = 0;
    while (1) {
        int bufsize = conn->readbuf_size - conn->read_pos;
        if (bufsize <= 0) {
            cerr<"read buffer not enough, fd:"<<conn->fd<<endl;
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

#endif //__NET_UTILS_H__

