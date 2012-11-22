#ifndef __FILE_SAVER_H__
#define __FILE_SAVER_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include<sys/stat.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

class FileSaver {
    public:
        FileSaver(const char *ftok_path) {
            ftok_path_ = ftok_path;
        }
        ~FileSaver() {
        }
        
        /*
        * filename: 不包括路径部分
        */
        bool initSem(const char* filename);

        /*
        * filename: 不包含路径部分
        * saved_len: 已经存储的长度
        * ... 
        * return: 0 需要重新save, -1 失败，l 写入多少数据
        */
        int save(const char* filename, int saved_len, char *buf, int buflen); 

    private:
        int semid_;
        string ftok_path_;

        void lock() {
            static sembuf op = {0, -1, SEM_UNDO};
            semop(semid_, &op, 1);
        }
        bool lock(int timeout) {
            static sembuf op = {0, -1, SEM_UNDO};
            timespec to = {timeout, 0};
            return semtimedop(semid_, &op, 1, &to) == 0;
        }
        void unlock() {
            static sembuf ulop = {0, 1, SEM_UNDO};
            semop(semid_, &ulop, 1);
        }
};

#endif

