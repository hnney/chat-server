#include "filesaver.h"

bool FileSaver::initSem(const char* filename) {
    string key_name = ftok_path_ + filename;
    int fd = open(key_name.c_str(), O_CREAT|O_RDWR, 0666);
    if (fd < 0 ) {
        cerr<<"open tokfile:"<<key_name<<" failed"<<" err:"<<errno<<endl;
        return false;
    }
    close(fd);
    key_t kt = ftok(key_name.c_str(), 1);
    if (kt < 0) {
        cerr<<"get key_t failed, key_name:"<<key_name.c_str()<<" err:"<<errno<<endl;
        return false;
    }

    int oflag = IPC_CREAT | IPC_EXCL | 0666;
    semun arg;
    if ((semid_ = semget(kt, 1, oflag)) >= 0) {
        arg.val = 1;
        semctl(semid_, 0, SETVAL, arg);
        cout<<"semaphore set val:"<<arg.val<<endl;
    }
    else if (errno == EEXIST) {
        semid_ = semget(kt, 1, 0);
        if (semid_ < 0) {
            cerr<<"semget failed, key_t:"<<kt<<endl;
            return false;
        }
        arg.val = semctl(semid_, 0, GETVAL);
        if (arg.val != 1) {
            cerr<<"semaphore val is not 1, val:"<<arg.val<<endl;
        }
    }
    return true;
}

int FileSaver::save(const char* filename, int saved_len, char *buf, int buflen) {
    if (filename == NULL || buf == NULL || buflen == 0) {
        return -1;
    }
    if(!lock(3)) {
        cerr<<"get lock failed"<<endl;
        return 0;
    }
    int fd = open(filename, O_CREAT|O_RDWR, 0666);
    if (fd < 0) {
        cerr<<"open file:"<<filename<<" failed, err:"<<errno<<endl;
        return -1;
    }
    int fl = lseek(fd, 0, SEEK_END);
    if (fl < 0) {
        cerr<<"lseek file:"<<filename<<" err:"<<errno<<endl;
        close(fd);
        return -1;
    }
    int wsize = 0;
    //TODO 重复收到
    if (fl > saved_len) {
        return -1;
    }
    else if (fl == saved_len) {
        int n = 0;
        for(;;) {
            n = write(fd, buf, buflen);
            if (n < 0) {
                cerr<<"write data to file:"<<filename<<" failed, err:"<<errno<<endl;
                break;
            }
            wsize += n;
            if (wsize >= buflen) {
                break;
            }
        }
    }
    close(fd);
    unlock();
    return wsize;
}

#ifdef __TEST__

int main(int argc, char *argv) {

    FileSaver f("/tmp/");
    f.initSem("11224.txt");
    while (1) {
        getchar();
        if (f.save("11224.txt", 0, "123", 3) == 3) {   
        }
        else {
            cerr<<"saved file failed"<<endl;
        }
    }

    return 0;
}
#endif

